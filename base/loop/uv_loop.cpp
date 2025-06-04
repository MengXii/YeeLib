/*
 * Copyright (c) 2023.12.05
 * Author: MengXii
 */

#include "uv_loop.h"

#include "glog/logging.h"

namespace base::loop {

void UvEpollLoop::OnEvent(uv_poll_t* handle, int status, int events) {
  if (__glibc_unlikely(status < 0)) {
    LOG(ERROR) << "uv poll failed, error: " << uv_strerror(status);
    return;
  }

  auto* handler = reinterpret_cast<EventHandler*>(handle->data);

  eventfd_t value;
  [[maybe_unused]] auto n = eventfd_read(handle->io_watcher.fd, &value);
  (handler->event_cb)(value);
}

void UvEpollLoop::AsyncStopHandle(uv_async_t* handle) {
  UvEpollLoop* loop = static_cast<UvEpollLoop*>(handle->data);
  uv_stop(loop->uv_loop());
}

UvEpollLoop::UvEpollLoop()
    : loop_(uv_default_loop()), async_stop_(new uv_async_t) {
  uv_loop_init(loop_);
  uv_async_init(loop_, async_stop_, AsyncStopHandle);
  async_stop_->data = this;
}

UvEpollLoop::~UvEpollLoop() {
  for (auto& handler : handlers_) {
    int efd = -1;
    CHECK_EQ(
        uv_fileno(reinterpret_cast<uv_handle_t*>(&handler.efd_watcher), &efd),
        0);
    uv_close(reinterpret_cast<uv_handle_t*>(&handler.efd_watcher), nullptr);
    close(efd);
  }
}

void UvEpollLoop::Run() { uv_run(loop_, UV_RUN_DEFAULT); }

void UvEpollLoop::Stop() { uv_async_send(async_stop_); }

void UvEpollLoop::WatchFd(int event_fd, const std::function<void(int)>& cb) {
  auto& handler = handlers_.emplace_back();
  CHECK_EQ(uv_poll_init(loop_, &(handler.efd_watcher), event_fd), 0);
  handler.event_cb = cb;
  handler.efd_watcher.data = &(handler);
  CHECK_EQ(uv_poll_start(&(handler.efd_watcher), uv_poll_event::UV_READABLE,
                         OnEvent),
           0);
}

}  // namespace base::loop