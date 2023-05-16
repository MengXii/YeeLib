/**
 * Copyright (c) 2023.05.11
 * Author: MengXii
 */

#pragma once
#include <glog/logging.h>
#include <sys/eventfd.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <tuple>
#include <type_traits>
#include <vector>

#include "base/event_loop.h"
#include "event2/event.h"

namespace {
static constexpr uint32_t kDefaultQueueSize = 8196;
}

namespace yeelib::base {

template <typename... Elem>
class EventHandler {
 public:
  virtual void OnEvent(Elem... elements) = 0;

 protected:
  virtual ~EventHandler() = default;
};

template <typename... Elem>
class EventQueue {
 public:
  EventQueue(event_base *event_base, EventHandler<Elem...> *handler,
             size_t size = kDefaultQueueSize);
  EventQueue(EventLoop *event_loop, EventHandler<Elem...> *handler,
             size_t size = kDefaultQueueSize);
  ~EventQueue();

  bool Empty() const;
  bool IsClosed() const;
  void Close();

  event_base *GetEventBase() const;

  template <typename... U>
  bool Push(U &&...elements);

 private:
  using ValueType = std::tuple<Elem...>;

  static void ReadCallback(evutil_socket_t fd, short event, void *contex);

  void Initialize();
  void HandleEvents();
  size_t TakeAllValue();
  void Callback(ValueType &&value);
  void CallbackInternal(Elem &&...elements);

 private:
  event_base *event_base_;
  event *event_;
  int event_fd_;
  EventHandler<Elem...> *handler_;
  std::atomic<bool> closed_;
  size_t max_size_;
  std::mutex mutex_;
  std::queue<ValueType> q_;
  std::vector<ValueType> values_;
};

}  // namespace yeelib::base

#include "base/event_queue_impl.h"