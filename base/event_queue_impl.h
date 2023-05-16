/*
 * Copyright (c) 2023.05.15
 * Author: MengXii
 */
#include "event_queue.h"
#include "gsl/gsl-lite.hpp"

namespace yeelib::base {

template <typename... Elem>
void EventQueue<Elem...>::ReadCallback(evutil_socket_t fd, short event,
                                       void *context) {
  (void)event;
  auto *p = reinterpret_cast<EventQueue<Elem...> *>(context);
  CHECK(fd == p->event_fd_);
  p->HandleEvents();
}

template <typename... Elem>
EventQueue<Elem...>::EventQueue(EventLoop *event_loop,
                                EventHandler<Elem...> *handler, size_t size)
    : EventQueue(event_loop->GetEventBase(), handler, size) {}

template <typename... Elem>
EventQueue<Elem...>::EventQueue(event_base *event_base,
                                EventHandler<Elem...> *handler, size_t size)
    : event_base_(event_base),
      handler_(handler),
      closed_(true),
      max_size_(size) {
  Initialize();
}

template <typename... Elem>
event_base *EventQueue<Elem...>::GetEventBase() const {
  return event_base_;
}

template <typename... Elem>
EventQueue<Elem...>::~EventQueue() {
  Close();
}

template <typename... Elem>
bool EventQueue<Elem...>::Empty() const {
  std::unique_lock<std::mutex> lock(mutex_);
  return q_.empty();
}

template <typename... Elem>
bool EventQueue<Elem...>::IsClosed() const {
  return event_ == nullptr;
}

template <typename... Elem>
void EventQueue<Elem...>::Close() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!q_.empty()) {
    LOG(WARNING) << " You should taking all the elements from this EventQueue!";
  }

  closed_ = true;

  ::event_del(event_);
  ::event_free(event_);
  ::close(event_fd_);
  event_ = nullptr;
}

template <typename... Elem>
template <typename... U>
bool EventQueue<Elem...>::Push(U &&...elements) {
  static_assert(std::is_convertible_v<std::tuple<U...>, std::tuple<Elem...>>,
                "Mismatched types for Push()");

  std::unique_lock<std::mutex> lock(mutex_);
  if (closed_) {
    LOG(WARNING)
        << " You should not push an element after this queue was closed!";
    return false;
  }

  if (q_.size() >= max_size_) {
    LOG(INFO) << "Too many packets in queue: " << q_.size();
    return false;
  }

  uint64_t increment = 1;
  ssize_t len = sizeof(uint64_t);
  // counter to increase
  if (::write(event_fd_, &increment, len) != len) {
    LOG(WARNING) << " Failed to write an increment into the event fd";
    return false;
  }

  q_.push(ValueType(std::forward<U>(elements)...));
  return true;
}

template <typename... Elem>
void EventQueue<Elem...>::Initialize() {
  closed_ = false;
  event_fd_ = eventfd(0, EFD_NONBLOCK);
  values_.reserve(max_size_);
  if (event_fd_ == -1) {
    LOG(FATAL) << "Failed to create an event fd: " << strerror(errno);
    return;
  }
  event_ = ::event_new(event_base_, event_fd_, EV_READ | EV_PERSIST,
                       ReadCallback, this);
  if (::event_add(event_, nullptr) != 0) {
    LOG(FATAL) << "Failed to add event_fd";
    return;
  }
}

template <typename... Elem>
void EventQueue<Elem...>::HandleEvents() {
  auto _ = gsl::finally([this] {
    std::vector<ValueType> clear;
    clear.reserve(max_size_);
    values_.swap(clear);
  });
  TakeAllValue();
  if (!handler_) {
    LOG(WARNING) << " EventQueue Handler Pointer is nullptr";
    return;
  }
  for (auto &value : values_) {
    Callback(std::move(value));
  }
}

template <typename... Elem>
size_t EventQueue<Elem...>::TakeAllValue() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (closed_) {
    LOG(WARNING) << " this EventQueue is close!";
    return 0;
  }

  int64_t len = sizeof(uint64_t);
  int64_t count = 0;
  int read = -1;

  // read counter from event_fd
  while ((read = ::read(event_fd_, &count, len)) == -1) {
    switch (errno) {
      case EWOULDBLOCK:  // empty
        return 0;
      case EINTR:
        continue;
      default:
        CHECK(false) << "unknown type";
        break;
    }
  }

  CHECK(read == len) << "Partial read from the event fd" << event_fd_ << " "
                     << strerror(errno);

  int64_t size = q_.size();
  if (size != count) {
    LOG(WARNING) << "The queue and read sizes are inconsistent";
  }

  while (!q_.empty()) {
    values_.emplace_back(std::move(q_.front()));
    q_.pop();
  }

  return size;
}

template <typename... Elem>
void EventQueue<Elem...>::CallbackInternal(Elem &&...elements) {
  handler_->OnEvent(std::move(elements)...);
}

template <typename... Elem>
void EventQueue<Elem...>::Callback(ValueType &&value) {
  std::apply(
      [this](auto &&...args) { CallbackInternal(std::forward<Elem>(args)...); },
      std::move(value));
}

}  // namespace yeelib::base