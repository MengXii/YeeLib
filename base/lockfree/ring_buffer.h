/**
 * Copyright (c) 2023.05.30
 * Author: MengXii
 */

#pragma once

#include <glog/logging.h>

#include <atomic>
#include <cstdint>

#include "base/cpp_feature.h"

namespace yeelib::base {

template <typename T>
class RingBuffer {
  static constexpr bool IS_POWER_OF_TWO(uint64_t x) {
    return x && ((x & (x - 1)) == 0);
  }

 public:
  constexpr static uint64_t CACHE_LINE_SIZE = 64;
  using ValueType = T;
  using Pointer = ValueType *;

  class EnqueueIterator {
   public:
    ALWAYS_INLINE EnqueueIterator() : ring_(nullptr), len_(0), pos_(0) {}

    ALWAYS_INLINE EnqueueIterator(RingBuffer *ring, uint64_t len)
        : ring_(ring), len_(len), pos_(0) {}

    ALWAYS_INLINE bool IsValid() const { return ring_ != nullptr && HasNext(); }

    ALWAYS_INLINE uint64_t Size() const { return len_; }

    ALWAYS_INLINE uint64_t IteratedPosition() const { return pos_; }

    ALWAYS_INLINE uint64_t NextTail() const {
      return (ring_->local_tail_ + pos_) & ring_->size_mask_;
    }

    ALWAYS_INLINE bool HasNext() const { return pos_ != len_; }

    ALWAYS_INLINE Pointer Next() {
      Pointer ret = reinterpret_cast<Pointer>(ring_->data_) +
                    ((ring_->local_tail_ + pos_) & ring_->size_mask_);
      pos_++;
      return ret;
    }

    ALWAYS_INLINE uint64_t ResetLocalTail() {
      ring_->local_tail_ = NextTail();
      len_ = len_ - pos_;
      pos_ = 0;
      return ring_->local_tail_;
    }

   private:
    RingBuffer *ring_;
    uint64_t len_;
    uint64_t pos_;

    friend class RingBuffer;
  };

  class DequeueIterator {
   public:
    ALWAYS_INLINE DequeueIterator() : ring_(nullptr), len_(0), pos_(0) {}

    ALWAYS_INLINE DequeueIterator(RingBuffer *ring, uint64_t len)
        : ring_(ring), len_(len), pos_(0) {}

    ALWAYS_INLINE bool IsValid() const { return ring_ != nullptr && HasNext(); }

    ALWAYS_INLINE uint64_t ReservedSize() const { return len_; }

    ALWAYS_INLINE uint64_t IteratedPosition() const { return pos_; }

    ALWAYS_INLINE uint64_t NextFront() const {
      return (ring_->local_front_ + pos_) & ring_->size_mask_;
    }

    ALWAYS_INLINE bool HasNext() const { return pos_ != len_; }

    ALWAYS_INLINE Pointer Next() {
      Pointer ret = reinterpret_cast<Pointer>(ring_->data_) +
                    ((ring_->local_front_ + pos_) & ring_->size_mask_);
      pos_++;
      return ret;
    }

    ALWAYS_INLINE uint64_t ResetLocalFront() {
      ring_->local_front_ = NextFront();
      len_ = len_ - pos_;
      pos_ = 0;
      return ring_->local_front_;
    }

   private:
    RingBuffer *ring_;
    uint64_t len_;
    uint64_t pos_;

    friend class RingBuffer;
  };

  static uint64_t RingSize(uint64_t block_size) {
    return sizeof(RingBuffer) + sizeof(ValueType) * block_size;
  }

  static RingBuffer *Create(uint64_t block_size) {
    uint64_t size = RingSize(block_size);
    return Create(malloc(size), block_size, size);
  }

  static RingBuffer *Create(void *addr, uint64_t block_size,
                            uint64_t addr_len) {
    uint64_t size = RingSize(block_size);
    if (addr_len < size) {
      return nullptr;
    }
    uint8_t *p = reinterpret_cast<uint8_t *>(addr);
    return new (p) RingBuffer(block_size);
  }

  static RingBuffer *Attach(void *addr, uint64_t block_size,
                            uint64_t addr_len) {
    uint64_t size = RingSize(block_size);
    if (addr_len < size) {
      return nullptr;
    }
    return reinterpret_cast<RingBuffer *>(addr);
  }

  ALWAYS_INLINE EnqueueIterator Reserve(uint64_t reserve) {
    uint64_t front = front_.load(std::memory_order_acquire);
    uint64_t element_num = (local_tail_ + block_size_ - front) & size_mask_;
    if (block_size_ - element_num > reserve) {
      return {this, reserve};
    } else {
      return {};
    }
  }

  ALWAYS_INLINE bool Commit(EnqueueIterator &reservation) {
    uint64_t next_tail = reservation.ResetLocalTail();
    std::atomic_thread_fence(
        std::memory_order_release);  // note()
                                     // 这里其实等价于tail_.store(next_tail,
                                     // std::memory_order_release);
    tail_.store(next_tail, std::memory_order_relaxed);
    return !reservation.HasNext();
  }

  ALWAYS_INLINE DequeueIterator DequeueAll() {
    uint64_t tail = tail_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    uint64_t element_num = (tail + block_size_ - local_front_) & size_mask_;
    return {this, element_num};
  }

  ALWAYS_INLINE bool ConfirmDeque(DequeueIterator &reservation) {
    uint64_t next_front = reservation.ResetLocalFront();
    std::atomic_thread_fence(std::memory_order_release);
    front_.store(next_front, std::memory_order_relaxed);
    return !reservation.HasNext();
  }

 private:
  explicit RingBuffer(uint64_t block_size)
      : front_(0),
        local_tail_(0),
        tail_(0),
        local_front_(0),
        block_size_(block_size),
        size_mask_(block_size_ - 1) {
    bool size_check = IS_POWER_OF_TWO(block_size_) && block_size_ > 1;
    CHECK(size_check);
  }

  ~RingBuffer() {}

 private:
  std::atomic_uint64_t front_;
  uint64_t local_tail_;
  uint8_t
      cacheline_filler_0[CACHE_LINE_SIZE - sizeof(uint64_t) - sizeof(uint64_t)];
  std::atomic_uint64_t tail_;
  uint64_t local_front_;
  uint8_t
      cacheline_filler_1[CACHE_LINE_SIZE - sizeof(uint64_t) - sizeof(uint64_t)];
  const uint64_t block_size_;
  const uint64_t size_mask_;

  uint8_t data_[];

  DISALLOW_COPY_AND_ASSIGN(RingBuffer);
};
}  // namespace yeelib::base
