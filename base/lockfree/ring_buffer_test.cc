/**
 * Copyright (c) 2024.01.30
 * Author: MengXii
 */

#include "ring_buffer.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace base::lockfree {

TEST(RING_BUFFER, MainTest) {
  char* a = new char[100];
  [[maybe_unused]] auto* ring = RingBuffer<int32_t>::Create(a, 5, 100);
  // Add your test logic here
  // ...
  delete[] a;  // Don't forget to free the allocated memory
}

}  // namespace base::lockfree

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
