/**
 * Copyright (c) 2023.06.13
 * Author: MengXii
 */

#pragma once

#include "network/tcp/normal_tcp_client.h"

#include <netinet/tcp.h>

#include "event2/bufferevent.h"
#include "event2/event.h"

namespace yeelib::network::tcp {

void ConnectCallback(bufferevent *bev, short events, void *context) {
  (void)bev;

  NormalTcpClient *p = reinterpret_cast<NormalTcpClient *>(context);
  assert(bev == p->pending_.get());

  p->OnConnect(events);
}

NormalTcpClient::NormalTcpClient(base::EventLoop *event_loop,
                                 const std::string &remote_ip,
                                 uint16_t remote_port,
                                 CacheMode cache_mode = kSocket)
    : event_loop_(event_loop),
      meta_{.remote_ip = remote_ip, .remote_port = remote_port},
      connecting_(false),
      stoped_(false),
      cache_mode_(cache_mode),
      max_cache_size_(0),
      socket_buffer_size_(0) {}

bool NormalTcpClient::SetSocketBufferSize(int32_t size) {
  socket_buffer_size_ = size;

  if (socket_buffer_size_ > 0 && conn_) {
    return conn_->SetSocketBufferSize(size);
  }

  return false;
}

void NormalTcpClient::SetCacheMode(CacheMode cache_mode, uint32_t cache_size) {
  cache_mode_ = cache_mode;
  cache_size_ = cache_size;

  if (conn_) {
    conn_->SetCacheMode(cache_mode, cache_size);
  }
}

}  // namespace yeelib::network::tcp