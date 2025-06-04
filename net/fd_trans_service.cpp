/**
 * Copyright (c) 2024.01.05
 * Author: MengXii
 */

#include "fd_trans_service.h"

#include <unistd.h>

#include <algorithm>

#include "glog/logging.h"
#include "uv.h"

namespace base::net {

/*
1. bind local port
2. recv fd
*/
void OnNewConnection(uv_stream_t *server, int status) {
  auto *service = reinterpret_cast<FdTransService *>(server->data);
  CHECK_NOTNULL(service);
  service->OnSockNewConnection(server, status);
}

void OnClientClose(uv_handle_t *client) {
  auto *service = reinterpret_cast<FdTransService *>(client->data);
  CHECK_NOTNULL(service);
  service->OnSockClientClose(client);
}

void AllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  constexpr static int kMaxBufSize = 2048;
  thread_local static char event_buf[kMaxBufSize];
  thread_local static uv_buf_t buf_obj{.base = event_buf,
                                       .len = sizeof(event_buf)};
  *buf = buf_obj;
}

void OnRead(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  auto *service = reinterpret_cast<FdTransService *>(client->data);
  CHECK_NOTNULL(service);
  service->OnSockRead(client, nread, buf);
}

FdTransService::FdTransService(base::loop::UvEpollLoop *loop)
    : loop_(loop), bound_(false), domain_socket_path_("") {
  wreq_.buf = uv_buf_init((char *)malloc(kBufSize), kBufSize);
}

bool FdTransService::BindDomainSocket(const std::string &sock_path) {
  ::unlink(sock_path.c_str());
  CHECK_EQ(uv_pipe_init(loop_->uv_loop(), &server_, 0), 0);
  server_.data = this;
  bound_ = true;
  CHECK_EQ(uv_pipe_bind(&server_, sock_path.c_str()), 0);
  CHECK_EQ(uv_listen(reinterpret_cast<uv_stream_t *>(&server_), kMaxConnections,
                     OnNewConnection),
           0);
  return true;
}

void FdTransService::OnSockNewConnection(uv_stream_t *server, int status) {
  if (__glibc_unlikely(status < 0)) {
    LOG(ERROR) << "Connect err " << uv_err_name(status);
  }
  auto &client = clients_.emplace_back();
  client.data = this;
  auto uv_pipe_init_r = uv_pipe_init(loop_->uv_loop(), &client, 0);
  CHECK_EQ(uv_pipe_init_r, 0);
  if (uv_accept(server, reinterpret_cast<uv_stream_t *>(&client)) != 0) {
    LOG(WARNING) << "Failed to " << __func__;
    uv_close(reinterpret_cast<uv_handle_t *>(&client), OnClientClose);
  }
  CHECK_EQ(uv_read_start(reinterpret_cast<uv_stream_t *>(&client), AllocBuffer,
                         OnRead),
           0);
}

void FdTransService::OnSockClientClose(uv_handle_t *client) {
  DCHECK(client != nullptr);
  auto iter = std::find_if(
      clients_.begin(), clients_.end(), [client](uv_pipe_t &stored_client) {
        return client == reinterpret_cast<uv_handle_t *>(&stored_client);
      });
  DCHECK(iter != clients_.end());
  clients_.erase(iter);
}

void FdTransService::OnSockRead(uv_stream_t *client, ssize_t nread,
                                const uv_buf_t *buf) {
  if (__glibc_unlikely(nread < 0)) {
    if (nread != UV_EOF) {
      LOG(ERROR) << "Read err " << uv_err_name(nread);
    }
    uv_close(reinterpret_cast<uv_handle_t *>(client), OnClientClose);
    return;
  }
  this->ReceiveFd(buf->base, nread, client);
}

bool FdTransService::SendFd(const std::string &remote, const std::string &data,
                            uv_stream_t *client) {
  CHECK_LT(data.size(), kBufSize)
      << "send fd result to remote: " << remote << " Failed";
  memcpy(wreq_.buf.base, data.data(), data.size());
  uv_write(&wreq_.write_req, client, &wreq_.buf, 1, nullptr);

  return true;
}

void FdTransService::ReceiveFd(char *buf, int32_t len, uv_stream_t *client) {
  FdMsg fd_msg;
  int32_t payload_len = *reinterpret_cast<int32_t *>(buf);
  CHECK(fd_msg.ParseFromArray(buf + sizeof(payload_len), payload_len));
  LOG(INFO) << __func__ << "\n" << fd_msg.DebugString();
  switch (fd_msg.type()) {
    case FdMsg::FD_REGISTER: {
      CHECK_EQ(fd_msg.fds().size(), 1);
      this->SetServiceFd(fd_msg.service_name(), fd_msg.fds()[0]);
    } break;
    case FdMsg::FD_QUERY: {
      FdMsg send_fd_msg;
      send_fd_msg.set_type(FdMsg::FD_RESULT);
      send_fd_msg.set_service_name("Monitor");
      for (auto &&service_name : fd_msg.querys()) {
        if (service_fds_.find(service_name) == service_fds_.end()) {
          send_fd_msg.add_fds(-1);
        } else {
          send_fd_msg.add_fds(service_fds_.at(service_name));
        }
      }
      std::string raw;
      [[maybe_unused]] auto success = send_fd_msg.SerializeToString(&raw);
      this->SendFd(fd_msg.service_name(), raw, client);
    } break;
    case FdMsg::FD_RESULT: {
    } break;
    default: {
      LOG(ERROR) << "no support fd msg type: " << fd_msg.type()
                 << " from service: " << fd_msg.service_name();
    } break;
  }
}

}  // namespace base::net
