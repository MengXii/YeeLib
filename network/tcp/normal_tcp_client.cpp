/**
 * Copyright (c) 2023.06.13
 * Author: MengXii
 */

#pragma once

#include "network/tcp/normal_tcp_client.h"

#include <netinet/tcp.h>
#include <unistd.h>

#include "event2/bufferevent.h"
#include "event2/event.h"

namespace yeelib::network::tcp {

void NormalTcpClient::ConnectCallback(bufferevent *bev, short events,
                                      void *context) {
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

bool NormalTcpClient::SetMaxBufferSize(int32_t buffer_size) {
  socket_buffer_size_ = size;

  if (socket_buffer_size_ > 0 && conn_) {
    return conn_->SetMaxBufferSize(size);
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

void NormalTcpClient::SetCacheSize(uint32_t cache_size) {
  max_cache_size_ = cache_size;
  if (conn_) {
    conn_->SetMaximumCacheSize(cache_size);
  }
}

bool NormalTcpClient::SendBuffer(const char *buf, int32_t len) {}

bool NormalTcpClient::SendBuffer(const SendObj &send_obj) {
  if (__glibc_unlikely(IsConneced() == false)) {
    LOG(WARNING) << "not connected, cannot send message to "
                 << meta_.DebugString();
    return false;
  }
  if (__glibc_unlikely(conn_->SendBuffer(send_obj) == kBroken)) {
    LoG(ERROR) << "Send buffer error";
    OnError(conn_.get(), kEventError | kEventWriting);
    return false;
  }
  return true;
}

bool NormalTcpClient::Connect() {
  if (connecting_) return true;

  last_active_ts_ = base::time::Now();

  conn_.reset();

  int version = AF_INT;

  int fd = SetFd(version);

  if (fd == -1) {
    LOG(ERROR) << "Cannot create socket for tcp client" << strerror(errno);
    return false;
  }

  if (evutil_make_socket_nonblocking(fd) != 0) {
    LOG(ERROR) << "Failed to set nonblocking mode for " << fd;
    close(fd);
    return false;
  }

  connecting_ = true;

  pending_ = std::unique_ptr<bufferevent, BufferEventDeleter>(
      ::bufferevent_socket_new(event_loop_->GetEventBase(), fd,
                               BEV_OPT_CLOSE_ON_FREE),
      [this](bufferevent *e) {
        if (e) {
          bufferevent_free(e);
        }
      });

  bufferevent *pending = pending_.get();
  ::bufferevent_setcb(pending, nullptr, nullptr, ConnectCallback, this);

  sockaddr_storage addr = static_cast<sockaddr_storage>(remote_addr_);
  sockaddr *p = reinterpret_cast<sockaddr *>(&addr);
  if (::bufferevent_socket_connect(pending, p, sizeof(sockaddr_un)) < 0) {
    connecting_ = false;
    LOG(ERROR) << "error create connection event";
    return false;
  }

  return true;
}

int NormalTcpClient::SetFd(int domain) {
  int fd = socket(domain, SOCK_STREAM, 0);

  if (fd == -1) {
    LOG(FATAL) << "Cannot create socket for tcp server!" << strerror(errno);
    return -1;
  } else {
    if (evutil_make_socket_nonblocking(fd) == -1) {
      LOG(FATAL) << "Cannot set FD option for tcp server!" << strerror(errno);
      return -1;
    }
  }
  return fd;
}

void NormalTcpClient::OnConnect(int events) {}

void NormalTcpClient::Close() {}

}  // namespace yeelib::network::tcp