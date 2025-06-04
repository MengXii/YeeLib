/**
 * Copyright (c) 2024.02.21
 * Author: MengXii
 */

#include "local_client.h"

#include "glog/logging.h"

namespace base::net {

void AllocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  constexpr static int kMaxBufSize = 100 * 2048;
  thread_local static char event_buf[kMaxBufSize];
  thread_local static uv_buf_t buf_obj{.base = event_buf,
                                       .len = sizeof(event_buf)};
  *buf = buf_obj;
}

void LocalClient::OnUvRead(uv_stream_t* client, ssize_t nread,
                           const uv_buf_t* buf) {
  auto* service = reinterpret_cast<LocalClient*>(client->data);
  CHECK_NOTNULL(service);
  service->OnRead(client, nread, buf);
}

void LocalClient::OnUvClientClose(uv_handle_t* client) {
  auto* service = reinterpret_cast<LocalClient*>(client->data);
  CHECK_NOTNULL(service);
  service->OnDisConnect(client);
}

void LocalClient::OnUvConnect(uv_connect_t* req, int status) {
  auto* service = reinterpret_cast<LocalClient*>(req->data);
  CHECK_NOTNULL(service);
  service->OnConnect(req, status);
}

LocalClient::LocalClient(base::loop::UvEpollLoop* loop,
                         const std::string& server_addr,
                         const std::string& service_name, uint32_t server_id)
    : loop_(loop),
      client_(nullptr),
      server_addr_(server_addr),
      service_name_(service_name),
      server_id_(server_id) {
  wreq_.buf = uv_buf_init((char*)malloc(kBufSize), kBufSize);
}

void LocalClient::Connect() {
  uv_pipe_init(loop_->uv_loop(), (uv_pipe_s*)&pipe_, 0);
  pipe_connect_.data = this;
  uv_pipe_connect(&pipe_connect_, (uv_pipe_s*)&pipe_, server_addr_.data(),
                  OnUvConnect);
}

void LocalClient::OnConnect(uv_connect_t* req, int status) {
  if (__glibc_unlikely(status < 0)) {
    LOG(WARNING) << "Connect to " << server_addr_ << " error!";
    free(req);
    return;
  }
  client_ = req;
  req->data = this;
  LOG(INFO) << __func__;
  uv_read_start((uv_stream_t*)req->handle, AllocBuffer, OnUvRead);
}

void LocalClient::OnDisConnect(uv_handle_t* client) {
  LOG(INFO) << "DisConnect.";
}

void LocalClient::OnRead(uv_stream_t* client, ssize_t nread,
                         const uv_buf_t* buf) {
  if (nread > 0) {
  } else if (nread < 0) {
    if (nread != UV_EOF) {
      LOG(ERROR) << "Read error " << uv_err_name(nread);
    }
    uv_close(reinterpret_cast<uv_handle_t*>(client), OnUvClientClose);
  }
}

int32_t LocalClient::SendData(const char* data, int32_t len) {
  CHECK_LT(len, kBufSize);
  *reinterpret_cast<int32_t*>(wreq_.buf.base) = len;
  memcpy(wreq_.buf.base + sizeof(int32_t), data, len);
  uv_write(&wreq_.write_req, client_->handle, &wreq_.buf, 1, nullptr);
  return len;
}

}  // namespace base::net
