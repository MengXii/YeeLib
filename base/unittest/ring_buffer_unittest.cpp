/**
 * Copyright (c) 2025.04.21
 * Author: MengXii
 */

#include "base/lockfree/ring_buffer.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <thread>

namespace yeelib::base::test {

struct RingBufferTestClass {
  static constexpr size_t PayloadSize = 1500;
  uint64_t size;
  uint64_t idx;
  uint8_t payload[PayloadSize];
};

static constexpr int StopIndex = 1000;
static constexpr int BatchSize = 10;
TEST(RingBufferTest, Base) {
  using SPSCQ = RingBuffer<RingBufferTestClass>;

  SPSCQ* q = SPSCQ::Create(1024 * 16);

  auto func = [&q]() {
    int last_idx = 0;
    while (true) {
      auto received_packets = q->DequeueAll();
      while (received_packets.HasNext()) {
        RingBufferTestClass a = *received_packets.Next();
        LOG(INFO) << "idx: " << a.idx;
        last_idx = a.idx;
      }
      q->ConfirmDeque(received_packets);
      if (last_idx == StopIndex * BatchSize) {
        break;
      }
    }
  };

  std::thread t(func);

  for (int i = 0; i < StopIndex; ++i) {
    auto iter = q->Reserve(BatchSize);
    int idx = i * BatchSize;
    sleep(0.01);
    if (iter.IsValid()) {
      while (iter.HasNext()) {
        auto next = iter.Next();
        next->size = 0;
        next->idx = ++idx;
        // memcpy(next->payload,data, data.size());
      }
      q->Commit(iter);
    }
  }

  t.join();
}

}  // namespace yeelib::base::test
