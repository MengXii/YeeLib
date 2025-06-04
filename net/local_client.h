/**
 * Copyright (c) 2024.02.21
 * Author: MengXii
 */

#include <string>

#include "base/loop/uv_loop.h"
#include "uv.h"

namespace base::net {

class ConnectClientCallback {};

class LocalClient {
 public:
  LocalClient(base::loop::UvEpollLoop* loop, const std::string& server_addr,
              const std::string& server_name, uint32_t server_id);

  void Connect();

  int32_t SendData(const std::string data) {
    return this->SendData(data.data(), data.size());
  }
  int32_t SendData(const char* data, int32_t len);

  const std::string service_name() { return service_name_;}

  static void OnUvRead(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
  static void OnUvClientClose(uv_handle_t* client);
  static void OnUvConnect(uv_connect_t* req, int status);

 protected:
  constexpr static int32_t kBufSize = 2048;
  void OnConnect(uv_connect_t* req, int status);
  void OnDisConnect(uv_handle_t* client);
  void OnRead(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);

 private:
  base::loop::UvEpollLoop* loop_;
  uv_pipe_t pipe_;
  uv_connect_t pipe_connect_;
  uv_connect_t* client_;
  struct write_req_t {
    uv_write_t write_req;
    uv_buf_t buf;
  };
  write_req_t wreq_;
  std::string server_addr_;
  std::string service_name_;
  uint32_t server_id_;
};

}  // namespace base::net
