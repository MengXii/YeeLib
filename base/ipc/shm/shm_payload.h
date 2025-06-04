/**
 * Copyright (c) 2024.08.20
 * Author: MengXii
 */

#include <cstdint>

#include "base/lockfree/ring_buffer.h"
namespace base::ipc::shm {

constexpr size_t kMaxPayloadSize = 1500;

struct ShmPayload {
  uint64_t payload_size;
  uint8_t payload[kMaxPayloadSize];

  ShmPayload() = default;
  // can only be copied
  ShmPayload(const ShmPayload &rhs) : payload_size(rhs.payload_size) {
    ::memcpy(payload, rhs.payload, payload_size);
  }

  ShmPayload &operator=(const ShmPayload &rhs) {
    if (this == &rhs) {
      return *this;
    }
    payload_size = rhs.payload_size;
    ::memcpy(payload, rhs.payload, payload_size);
    return *this;
  }

  ShmPayload(ShmPayload &&) = delete;
  ShmPayload &operator=(ShmPayload &&) = delete;
};

using SPSCQ = lockfree::RingBuffer<ShmPayload>;

}  // namespace base::ipc::shm
