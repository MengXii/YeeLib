/**
 * Copyright (c) 2023 Presto Labs Pte. Ltd.
 * Author: Ju
 */

#pragma once

#include <unistd.h>

#include <functional>
#include <mutex>
#include <queue>
#include <tuple>
#include <vector>

#include "base/lockfree/lock_free_queue.h"
#include "base/loop/uv_loop.h"
#include "glog/logging.h"
namespace base::loop {
constexpr static int32_t kMaxSize = 16;

template <typename... Elem>
class UvEpoolEventQueue {
 public:
  using ValueType = std::tuple<Elem...>;
  using EventHandler = std::function<void(Elem...)>;
  UvEpoolEventQueue(UvEpollLoop* loop, EventHandler handler)
      : loop_(loop), handler_(handler) {
    event_fd_ = eventfd(0, EFD_NONBLOCK);
    loop_->WatchFd(event_fd_, std::bind(&UvEpoolEventQueue::OnEvent, this,
                                        std::placeholders::_1));
  }
  ~UvEpoolEventQueue() { close(event_fd_); }

  template <typename... U>
  bool Push(U&&... elements) {
    static_assert(std::is_convertible_v<std::tuple<U...>, std::tuple<Elem...>>,
                  "Mismatched types for Push()");

    std::unique_lock<std::mutex> lock(mutex_);
    if (q_.size() >= kMaxSize) {
      LOG(INFO) << "Too many packets in queue: " << q_.size();
      return false;
    }
    q_.push(ValueType(std::forward<U>(elements)...));

    auto rc = eventfd_write(event_fd_, 1lu);
    if (__glibc_unlikely(rc < 0)) {
      LOG(WARNING) << " Failed to write an increment into the event fd";
      return false;
    }
    return true;
  }

 private:
  void OnEvent(int event_count) {
    std::vector<ValueType> events;
    TakeAll(&events, event_count);
    if (__glibc_unlikely(handler_ == nullptr)) {
      return;
    }
    for (auto& value : events) {
      std::apply(
          [this](auto&&... args) { handler_(std::forward<Elem>(args)...); },
          std::move(value));
    }
  }

  int32_t TakeAll(std::vector<ValueType>* events, int event_count) {
    std::unique_lock<std::mutex> lk(mutex_);
    if (event_count > 0) {
      int32_t q_size = q_.size();
      if (q_size != event_count) {
        LOG(WARNING) << "Inconsistent data: " << q_size << "," << event_count;
      }
      events->reserve(q_size);

      while (!q_.empty()) {
        events->push_back(std::move(q_.front()));
        q_.pop();
      }
      return q_size;
    }
    return 0;
  }

 protected:
  UvEpollLoop* loop_;
  int event_fd_;
  EventHandler handler_;
  std::mutex mutex_;
  std::queue<ValueType> q_;
};

template <typename... Elem>
class LockFreeEventQueue {
 public:
  using ValueType = std::tuple<Elem...>;
  using EventHandler = std::function<void(Elem...)>;
  LockFreeEventQueue(UvEpollLoop* loop, EventHandler handler)
      : loop_(loop), handler_(handler) {
    event_fd_ = eventfd(0, EFD_NONBLOCK);
    loop_->WatchFd(event_fd_, std::bind(&LockFreeEventQueue::OnEvent, this,
                                        std::placeholders::_1));
  }

  ~LockFreeEventQueue() { close(event_fd_); }

  template <typename... U>
  bool Push(U&&... elements) {
    static_assert(std::is_convertible_v<std::tuple<U...>, std::tuple<Elem...>>,
                  "Mismatched types for Push()");

    if (q_.size() >= kMaxSize) {
      LOG(INFO) << "Too many packets in queue: " << q_.size();
      return false;
    }
    q_.push(ValueType(std::forward<U>(elements)...));
    auto rc = eventfd_write(event_fd_, 1lu);
    if (__glibc_unlikely(rc < 0)) {
      LOG(WARNING) << " Failed to write an increment into the event fd";
      return false;
    }
    return true;
  }

 private:
  void OnEvent(int event_count) {
    std::vector<ValueType> events;
    TakeAll(&events, event_count);

    if (__glibc_unlikely(handler_ == nullptr)) {
      return;
    }
    for (auto& value : events) {
      std::apply(
          [this](auto&&... args) { handler_(std::forward<Elem>(args)...); },
          std::move(value));
    }
  }

  void TakeAll(std::vector<ValueType>* events, int event_count) {
    auto head = q_.pop_all();
    ValueType* value = nullptr;
    while ((value = head.front()) != nullptr) {
      events->push_back(std::move(*value));
      head.pop();
    }
  }

 protected:
  UvEpollLoop* loop_;
  int event_fd_;
  EventHandler handler_;
  base::lockfree::Queue<ValueType> q_;
};

}  // namespace base::loop
