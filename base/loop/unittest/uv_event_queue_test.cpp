/**
 * Copyright (c) 2023 Presto Labs Pte. Ltd.
 * Author: Ju
 */

#include "base/loop/uv_event_queue.h"

#include <atomic>
#include <thread>

#include "base/loop/uv_loop.h"
#include "glog/logging.h"
#include "gsl/gsl-lite.hpp"
#include "gtest/gtest.h"

namespace base::loop::testing {

TEST(UV_EPOOL_EVENT_QUEUE_TEST, data) {
  UvEpollLoop* loop = new UvEpollLoop();
  UvEpoolEventQueue<int32_t, std::string>::EventHandler handler =
      [&](int32_t a, std::string b) { LOG(INFO) << "data:" << a << ":" << b; };
  UvEpoolEventQueue<int32_t, std::string>* q =
      new UvEpoolEventQueue<int32_t, std::string>(loop, handler);

  auto thread_functor = [loop]() {
    LOG(INFO) << "Thread started";
    loop->Run();
    LOG(INFO) << "Thread exiting";
  };
  std::thread t1(thread_functor);
  int32_t i = 0;
  while (i < 5) {
    q->Push(i, std::to_string(i));
    ++i;
    sleep(1);
  }
  loop->Stop();
  t1.join();

  delete q;
  delete loop;
}

TEST(UV_EPOOL_EVENT_QUEUE_TEST, task) {
  UvEpollLoop* loop = new UvEpollLoop();

  using Task = std::function<void()>;

  UvEpoolEventQueue<Task>::EventHandler handler = [](Task task) { task(); };
  UvEpoolEventQueue<Task>* q = new UvEpoolEventQueue<Task>(loop, handler);

  std::thread t1([loop]() { return loop->Run(); });
  std::thread t2([q, loop]() {
    int32_t i = 0;
    while (true) {
      q->Push([i]() { LOG(INFO) << "task: " << i; });
      ++i;
      sleep(1);

      if (i == 5) {
        break;
      }
    }
    loop->Stop();
  });

  t1.join();
  t2.join();

  delete q;
  delete loop;
}

TEST(LOCK_FREE_QUEUE_EVENT_QUEUE_TEST, task) {
  UvEpollLoop* loop = new UvEpollLoop();

  using Task = std::function<void()>;

  LockFreeEventQueue<Task>::EventHandler handler = [](Task task) { task(); };
  LockFreeEventQueue<Task>* q = new LockFreeEventQueue<Task>(loop, handler);

  std::thread t1([loop]() { return loop->Run(); });
  std::thread t2([q, loop]() {
    int32_t i = 0;
    while (true) {
      q->Push([i]() { LOG(INFO) << "task: " << i; });
      ++i;
      sleep(1);

      if (i == 5) {
        break;
      }
    }
    loop->Stop();
  });

  t1.join();
  t2.join();

  delete q;
  delete loop;
}

}  // namespace base::loop::testing

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
