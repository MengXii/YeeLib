/*
 * Copyright (c) 2023.12.05
 * Author: MengXii
 */

#include <sys/eventfd.h>
#include <uv.h>

#include <functional>
#include <list>

#pragma once

namespace base::loop {
class UvEpollLoop {
 public:
  UvEpollLoop();
  UvEpollLoop(uv_loop_t* loop);
  ~UvEpollLoop();

  uv_loop_t* uv_loop() { return loop_; }

  static void AsyncStopHandle(uv_async_t* handle);

  void Run();
  void Stop();

  void WatchFd(int event_fd, const std::function<void(int)>& cb);

 protected:
  struct EventHandler {
    uv_poll_t efd_watcher;
    std::function<void(int)> event_cb;
  };

  static void OnEvent(uv_poll_t* handle, int status, int events);

 private:
  uv_loop_t* loop_{nullptr};
  uv_async_t* async_stop_;
  std::list<EventHandler> handlers_;
};
}  // namespace base::loop
