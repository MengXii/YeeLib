/*
 * Copyright (c) 2023.05.15
 * Author: MengXii
 */

#include "base/event_queue.h"

#include <gtest/gtest.h>
#include <glog/logging.h>

#include <string>
#include <thread>

#include "base/event_loop.h"

namespace yeelib::base::test {

static constexpr int32_t kStop = 10;

struct MyHandler : public base::EventHandler<int32_t, std::string> {
  MyHandler(EventLoop *loop) : loop_(loop) {}
  void OnEvent(int32_t i, std::string s) override {
    LOG(INFO) << i << " " << s;
    if (++count_ == kStop) {
      loop_->Stop();
    }
  }

  ~MyHandler() {}

  EventLoop *loop_;
  int count_{0};
};

TEST(EventQueueTest, Base) {
  using Queue = EventQueue<int32_t, std::string>;
  EventLoop *loop = new EventLoop();
  MyHandler h(loop);
  Queue q(loop->GetEventBase(), &h);

  auto thread_functor = [loop]() { return loop->Run(); };

  std::thread t(thread_functor);

  for (int32_t i = 0; i < kStop; ++i) {
    q.Push(i, std::to_string(i));
  }

  t.join();
}
}  // namespace yeelib::base::test
