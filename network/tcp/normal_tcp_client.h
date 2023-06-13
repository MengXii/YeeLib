/**
 * Copyright (c) 2023.06.12
 * Author: MengXii
 */

#pragma once

#include <functional>

#include "absl/status/status.h"
#include "base/cpp_feature.h"
#include "base/event_loop.h"
#include "network/client.h"
#include "network/tcp/normal_tcp_client.h"

namespace yeelib::network::tcp {

using namespace yeelib::network;

class NormalTcpClient : public ClientBase<NormalTcpClient> {
 public:
  using Base = ClientBase<NormalTcpClient>;
  using BufferEventDeleter = std::function<void(bufferevent *e)>;

  NormalTcpClient(base::EventLoop *event_loop, const std::string &remote_ip,
                  uint16_t remote_port, CacheMode cache_mode);

  void BindLocal(const std::string &local_ip, uint16_t local_port);

  bool SetMaxBufferSize(int32_t buffer_size);
  void SetCacheSize(uint32_t cache_size);
  void SetCacheMode(network::CacheMode mode, uint32_t cache_size);

  bool SendBuffer(const char *buf, int32_t len);

  bool Connect();
  bool Close();

  uint32_t GetServerHost() const override { return meta_.remote_ip; }
  uint16_t GetServerPort() const override { return meta_.remote_port; }
  uint32_t GetClientHost() const override { return meta_.local_ip; }
  uint16_t GetClientPort() const override { return meta_.local_port; }

 protected:
  friend Base;

  template <Forwarder>
  absl::Status DoConnectFast(std::string_view ip, uint32_t port,
                             Forwarder &forwarder) {}

 private:
  static void ConnectCallback(bufferevent *bev,
                              short events,  // NOLINT(runtime/int)
                              void *context);
  int SetFd(int domain);

  void OnConnect(int events);
  void CloseInternal();

 private:
  Base::EventLoop *event_loop_;

  network::MetaDataSocket meta_;
  std::unique_ptr<bufferevent, BufferEventDeleter> pending_;

  NormalTcpConnectionPtr conn_;

  bool connecting_;
  bool stoped_;

  CacheMode cache_mode_;
  uint32_t cache_mode_;
  uint32_t max_cache_size_;
  int32_t socket_buffer_size_;
};

}  // namespace yeelib::network::tcp
