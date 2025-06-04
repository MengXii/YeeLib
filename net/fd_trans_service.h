/**
 * Copyright (c) 2024.01.05
 * Author: MengXii
 */
#pragma once

#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>

#include "base/loop/uv_loop.h"
#include "base/net/fd_types.pb.h"

namespace base::net {

class FdTransService {
 public:
  using FdMsg = base::net::FdMsg;
  FdTransService(base::loop::UvEpollLoop* loop);

  bool BindDomainSocket(const std::string& sock_path);
  bool SendFd(const std::string& remote, const std::string& data,
              uv_stream_t* client);
  bool Bound() const { return bound_; }

  void ReceiveFd(char* buf, int32_t len, uv_stream_t* client);
  void OnSockNewConnection(uv_stream_t* server, int status);
  void OnSockClientClose(uv_handle_t* client);
  void OnSockRead(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);

  void SetServiceFd(const std::string& service_name, int32_t fd) {
    service_fds_.emplace(service_name, fd);
  }

 protected:
  constexpr static int32_t kBufSize = 2048;
  static const int32_t kMaxConnections = 16;

 private:
  base::loop::UvEpollLoop* loop_;
  uv_pipe_t server_{};
  bool bound_;
  std::string domain_socket_path_;
  std::list<uv_pipe_t> clients_;

  std::unordered_map<std::string, int32_t> service_fds_;
  struct write_req_t {
    uv_write_t write_req;
    uv_buf_t buf;
  };
  write_req_t wreq_;
};

}  // namespace base::net
