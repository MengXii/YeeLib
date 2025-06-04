/**
 * Copyright (c) 2024.09.23
 * Author: MengXii
 */

#include "shm_dispatcher.h"

#include <sys/fcntl.h>

namespace base::ipc::shm {

ShmDispatcher::ShmDispatcher(int32_t event_fd) : event_fd_(event_fd) {}

bool ShmDispatcher::SetEventfdNonblocking() {
  if (event_fd_ == -1) {
    LOG(WARNING) << __func__ << " error, fd = -1";
    return false;
  }

  int r = fcntl(event_fd_, F_SETFL, O_NONBLOCK);
  if (r == -1) {
    LOG(ERROR) << "Set domain socket nonblocking failed, errno = "
               << strerror(errno);
    return false;
  }
  return true;
}

SPSCQ* ShmDispatcher::CreateSPSCRingBuffer(const std::string& name,
                                           int32_t block_size, bool attach) {
  if (attach) {
    shm_base_ = std::make_unique<ShmBase>(AttachShmService(name, block_size));
  } else {
    shm_base_ = std::make_unique<ShmBase>(CreateShmService(name, block_size));
  }
  if (!shm_base_->Init()) {
    LOG(FATAL) << "Shm initialization failed";
    return nullptr;
  }

  void* addr = shm_base_->GetSharedAddr();
  SPSCQ* packet_ring = nullptr;
  if (attach) {
    packet_ring = SPSCQ::Create(addr, block_size, SPSCQ::RingSize(block_size));
  } else {
    packet_ring = SPSCQ::Attach(addr, block_size, SPSCQ::RingSize(block_size));
  }

  if (packet_ring == nullptr) {
    LOG(FATAL) << "not enough on creation, need size: "
               << SPSCQ::RingSize(block_size);
  }
  return packet_ring;
}

}  // namespace base::ipc::shm
