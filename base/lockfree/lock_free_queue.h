/**
 * Copyright (c) 2024.01.03
 * Author: MengXii
 */

#pragma once

#include <assert.h>

#include <atomic>

namespace base::lockfree {

template <typename Element>
class Queue {
  struct SSListEntry {
    SSListEntry *next;
    char data[];
  };

  class SSList {
   public:
    SSList(SSListEntry *head) { head_ = head; }

    bool empty() const { return head_ == nullptr; }

    void pop() {
      assert(!this->empty());

      SSListEntry *cur = head_;
      head_ = cur->next;

      Element *e = reinterpret_cast<Element *>(cur->data);
      e->~Element();
      void *addr = reinterpret_cast<void *>(cur);
      std::free(addr);
    }

    Element *front() {
      Element *e = nullptr;
      if (!this->empty()) {
        e = reinterpret_cast<Element *>(head_->data);
      }
      return e;
    }

   private:
    SSListEntry *head_;
  };

 public:
  Queue() {
    head_.store(nullptr, std::memory_order_relaxed);
    size_.store(0, std::memory_order_relaxed);
  }

  bool empty() const { return size_.load() == 0; }

  int32_t size() const { return size_.load(); }

  template <typename... Args>
  bool push(Args &&...args) {
    constexpr size_t size = sizeof(SSListEntry) + sizeof(Element);
    void *addr = std::malloc(size);
    SSListEntry *new_head = static_cast<SSListEntry *>(addr);
    new (new_head->data) Element(std::forward<Args>(args)...);
    auto success = this->CompareExchangeHead(new_head);
    if (__glibc_likely(success)) {
      size_.fetch_add(1, std::memory_order_relaxed);
    }
    return success;
  }

  SSList pop_all() {
    SSListEntry *new_head = nullptr;
    SSListEntry *old_head = head_.exchange(new_head);
    SSListEntry *rhead = nullptr;
    while (old_head != nullptr) {
      auto pre = old_head;
      old_head = pre->next;
      pre->next = rhead;
      rhead = pre;
    }
    size_.store(0, std::memory_order_relaxed);
    return SSList(rhead);
  }

 private:
  bool CompareExchangeHead(SSListEntry *new_head) {
    // Initialize the current old head
    SSListEntry *old_head = head_.load(std::memory_order_relaxed);
    // Standard compare-exchange loop, see
    // http://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange
    new_head->next = old_head;
    return head_.compare_exchange_weak(
        old_head,  // Note: updated in case of failure
        new_head,
        std::memory_order_release,   // memory synchronization in case of
                                     // success
        std::memory_order_relaxed);  // memory synchronization in case of
                                     // failure
  }

  std::atomic<SSListEntry *> head_;
  std::atomic<int32_t> size_;
};

}  // namespace base::lockfree
