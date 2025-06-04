/**
 * Copyright (c) 2024.09.23
 * Author: MengXii
 */

#include "base/ipc/shm/shm_base.h"
#include "base/ipc/shm/shm_payload.h"
#include "base/lockfree/ring_buffer.h"

namespace base::ipc::shm {

class ShmDispatcher {
 public:
  ShmDispatcher(int32_t event_fd);

 protected:
  bool SetEventfdNonblocking();
  SPSCQ* CreateSPSCRingBuffer(const std::string& name, int32_t block_size,
                              bool attach);

 private:
  int32_t event_fd_;
  std::unique_ptr<ShmBase> shm_base_;
};

}  // namespace base::ipc::shm
