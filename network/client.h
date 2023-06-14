/**
 * Copyright (c) 2023.06.12
 * Author: MengXii
 */

#pragma once

#include <glog/logging.h>

#include "absl/status/status.h"

namespace yeelib::network {

enum MetaDataType : uint8_t { kNone = 0, kSocket = 1 };

struct SendObj {
  struct Raw {
    const char *payload;
    size_t payload_len;
  }
};

enum IpVersion {
  kIpv4 = 1,
  kUnix = 2,
};

struct MetaDataSocket {
  std::string_view remore_ip;
  uint16_t remote_port{0};
  std::string_view local_ip;
  uint16_t local_port{0};
};

enum CacheMode : uint8_t {
  kDirect = 1,
  kBuffered = 2,
};

template <class Client>
class ClientBase {
 public:
  template <Forwarder>
  absl::Status Connect(std::string_view ip, uint32_t port,
                       Forwarder &forwarder) {
    return static_cast<Client *>(this)->DoConnectFast(ip, port, forwarder);
  }

  absl::Status DisConnect() {
    return static_cast<Client *>(this)->DisConnect();
  }

  virtual int32_t GetServerHost() const = 0;
  virtual int32_t GetServerPort() const = 0;
  virtual int32_t GetClientHost() const = 0;
  virtual int32_t GetClientPort() const = 0;
};

}  // namespace yeelib::network