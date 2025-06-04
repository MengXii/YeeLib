/**
 * Copyright (c) 2024.10.11
 * Author: MengXii
 */

#include "shm_sender.h"

namespace base::ipc::shm {

ShmSender::ShmSender(ShmSenderContext ctx, int32_t event_fd,
                     const std::string& shm_name, bool is_transporter)
    : ShmDispatcher(event_fd), ctx_(ctx), shm_name_(shm_name) {
  packet_tx_ring_ = CreateSPSCRingBuffer(shm_name, ctx_.shm_queue_block_size(),
                                         is_transporter);
}

void ShmSender::SendPacket(const std::string& packet) {}

bool ShmSender::WakeUp() {
    
}

}  // namespace base::ipc::shm
