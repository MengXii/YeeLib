/**
 * Copyright (c) 2023.05.11
 * Author: MengXii
 */

#include "base/event_loop.h"

#include <glog/logging.h>

#include <csignal>

namespace {
static void EventLog(int severity, const char *msg) {
  switch (severity) {
    case EVENT_LOG_DEBUG:
      LOG(INFO) << msg;
      break;
    case EVENT_LOG_MSG:
      LOG(INFO) << msg;
      break;
    case EVENT_LOG_WARN:
      LOG(WARNING) << msg;
      break;
    case EVENT_LOG_ERR:
      LOG(ERROR) << msg;
      break;
    default:
      break;
  }
}
}  // namespace

namespace yeelib::base {

EventLoop::EventLoop() : event_base_(::event_base_new()) {
  if (event_base_ == nullptr) {
    LOG(FATAL) << "Failed to create event base";
  }
}

EventLoop::~EventLoop() {
  if (event_base_) {
    ::event_base_free(event_base_);
    event_base_ = nullptr;
  }
}

bool EventLoop::Initialize() {
  // not to terminate
  if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
    LOG(ERROR) << "Ignore SIGHUP failed.";
    return false;
  }

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    LOG(ERROR) << "Ignore SIGPIPE failed.";
    return false;
  }

  ::event_set_log_callback(&EventLog);
  return true;
}

int EventLoop::Run() {
  if (!Initialize()) {
    return -1;
  }
  return ::event_base_dispatch(event_base_);
};

int EventLoop::Stop(bool immediately) {
  if (!event_base_) {
    return 0;
  }

  if (immediately) {
    return ::event_base_loopbreak(event_base_);
  }
  return ::event_base_loopexit(event_base_, nullptr);
}

event_base *EventLoop::GetEventBase() const { return event_base_; }

}  // namespace yeelib::base
