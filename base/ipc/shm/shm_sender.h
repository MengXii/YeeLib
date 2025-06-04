/**
 * Copyright (c) 2024.10.11
 * Author: MengXii
 */

#include "shm_contexts.pb.h"
#include "shm_dispatcher.h"

namespace base::ipc::shm {

class ShmSender : public ShmDispatcher {
 public:
  ShmSender(ShmSenderContext ctx, int32_t event_fd, const std::string &shm_name,
            bool is_transporter);

  void SendPacket(const std::string &packet);

 private:
  ShmSenderContext ctx_;
  const std::string &shm_name_;

  std::vector<std::string> buffers_to_send_;

  SPSCQ *packet_tx_ring_;
};

}  // namespace base::ipc::shm
