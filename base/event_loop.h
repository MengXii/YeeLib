/**
 * Copyright (c) 2023.05.11
 * Author: MengXii
 */
#pragma once

#include <event2/event.h>

namespace yeelib::base {

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  // 0 -> success
  // -1 -> error
  int Run();

  int Stop(bool immediately = false);

  event_base *GetEventBase() const;

 private:
  bool Initialize();

 private:
  event_base *event_base_;
};

}  // namespace ova::base