/**
 * Copyright (c) 2024.01.30
 * Author: MengXii
 */

#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include "glog/logging.h"

namespace base {
namespace lockfree {

template <typename T>
class RingBuffer {
 public:
  constexpr static uint64_t CACHE_LINE_SIZE = 64;
  using value_type = T;
  using pointer = value_type *;

  class EnqueueIterator {
   public:
    [[always_inline]] EnqueueIterator() : ring_(nullptr), len_(0), pos_(0) {}

    [[always_inline]] EnqueueIterator(RingBuffer *ring, uint64_t len)
        : ring_(ring), len_(len), pos_(0) {}

    [[always_inline]] bool IsValid() const {
      return ring_ != nullptr && HasNext();
    }

    [[always_inline]] uint64_t ReservedSize() const { return len_; }

    [[always_inline]] uint64_t IteratedSize() const { return pos_; }

    [[always_inline]] uint64_t NextTail() const {
      return (ring_->local_tail_ + pos_) & ring_->size_mask_;
    }

    [[always_inline]] bool HasNext() const { return pos_ != len_; }

    [[always_inline]] pointer Next() {
      pointer ret = reinterpret_cast<pointer>(ring_->data_) +
                    ((ring_->local_tail_ + pos_) & ring_->size_mask_);
      pos_++;
      return ret;
    }

    [[always_inline]] uint64_t ResetLocalTail() {
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
    [[always_inline]] DequeueIterator() : ring_(nullptr), len_(0), pos_(0) {}

    [[always_inline]] DequeueIterator(RingBuffer *ring, uint64_t len)
        : ring_(ring), len_(len), pos_(0) {}

    [[always_inline]] bool IsValid() const {
      return ring_ != nullptr && HasNext();
    }

    [[always_inline]] uint64_t ReservedSize() const { return len_; }

    [[always_inline]] uint64_t IteratedSize() const { return pos_; }

    [[always_inline]] uint64_t NextFront() const {
      return (ring_->local_front_ + pos_) & ring_->size_mask_;
    }

    [[always_inline]] bool HasNext() const { return pos_ != len_; }

    [[always_inline]] pointer Next() {
      pointer ret = reinterpret_cast<pointer>(ring_->data_) +
                    ((ring_->local_front_ + pos_) & ring_->size_mask_);
      pos_++;
      return ret;
    }

    [[always_inline]] uint64_t ResetLocalFront() {
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
    // Calculate enough memory for the block itself, as well as all the
    // elements it will contain
    return sizeof(RingBuffer) + sizeof(value_type) * block_size;
  }

  static RingBuffer *Create(void *addr, uint64_t block_size,
                            uint64_t addr_len) {
    uint64_t size = RingSize(block_size);
    if (addr_len < size) {
      return nullptr;
    }
    uint8_t *p = static_cast<uint8_t *>(addr);
    return new (p) RingBuffer(block_size);
  }

  static RingBuffer *Attach(void *addr, uint64_t block_size,
                            uint64_t addr_len) {
    uint64_t size = RingSize(block_size);
    if (addr_len < size) {
      return nullptr;
    }
    return static_cast<RingBuffer *>(addr);
  }

  // reserve and commit can only be called in producer
  // and must be paired up
  [[always_inline]] EnqueueIterator Reserve(uint64_t reserve) {
    uint64_t front = front_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    uint64_t element_num = (local_tail_ + block_size_ - front) & size_mask_;
    if (block_size_ - element_num > reserve) {
      return {this, reserve};
    } else {
      return {};
    }
  }

  // return true if reservation doesn't have remaining
  [[always_inline]] bool Commit(EnqueueIterator &reservation) {  // NOLINT
    uint64_t next_tail = reservation.ResetLocalTail();
    std::atomic_thread_fence(std::memory_order_release);
    tail_ = next_tail;
    return !reservation.HasNext();
  }

  // must be paired up with confirm deque
  [[always_inline]] DequeueIterator DequeueAll() {
    uint64_t tail = tail_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    // only intersted in used
    uint64_t element_num = (tail + block_size_ - local_front_) & size_mask_;
    return {this, element_num};
  }

  // return true if reservation doesn't have remaining
  [[always_inline]] bool ConfirmDeque(DequeueIterator &reservation) {  // NOLINT
    uint64_t next_front = reservation.ResetLocalFront();
    std::atomic_thread_fence(std::memory_order_release);
    front_ = next_front;
    return !reservation.HasNext();
  }

  // Returns the approximate number of items currently in the queue.
  // Safe to call from both the producer and consumer threads.
  inline size_t SizeApprox() const {
    uint64_t front = front_.load();
    uint64_t tail = tail_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    return (tail - front) & size_mask_;
  }

  // Note(chen): only for debug
  uint64_t GetFront() {
    uint64_t front = front_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    return front;
  }

  uint64_t GetTail() {
    uint64_t tail = tail_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    return tail;
  }

 private:
  explicit RingBuffer(uint64_t block_size)
      : front_(0),
        local_tail_(0),
        tail_(0),
        local_front_(0),
        block_size_(block_size),
        size_mask_(block_size_ - 1) {
    // Make sure the reader/writer threads will have the initialized memory
    // setup above:
    CHECK(__builtin_popcountll(block_size_));
    std::atomic_thread_fence(std::memory_order_seq_cst);
  }

  // Note: The queue should not be accessed concurrently while it's
  // being deleted. It's up to the user to synchronize this.
  ~RingBuffer() {
    // Make sure we get the latest version of all variables from other CPUs:
    std::atomic_thread_fence(std::memory_order_seq_cst);
    // todo(chen);
  }

 private:
  // Avoid false-sharing by putting highly contended variables on their own
  // cache lines
  std::atomic<uint64_t> front_;  // (Atomic) Elements are read from here
  uint64_t local_tail_;  // An uncontended shadow copy of tail, owned by the
                         // consumer

  uint8_t cacheline_filler0_[CACHE_LINE_SIZE -
                             sizeof(std::atomic<uint64_t>) -
                             sizeof(uint64_t)];
  std::atomic<uint64_t> tail_;  // (Atomic) Elements are enqueued here
  uint64_t local_front_;

  uint8_t
      cacheline_filler1_[CACHE_LINE_SIZE - sizeof(std::atomic<uint64_t>) -
                         sizeof(uint64_t)];  // next isn't very contended, but
                                             // we don't want it on the same
                                             // cache line as tail (which is)
  const uint64_t block_size_;
  const uint64_t size_mask_;

  uint8_t data_[];
};

}  // namespace lockfree
}  // namespace base
