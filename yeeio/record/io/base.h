/*
 * Copyright (c) 2023.03.06
 * Author: MengXii
 */

#include "absl/status/status.h"

namespace yeeio::io {

template <typename DataType>
class Reader {
 public:
  virtual ~Reader() = default;
  virtual absl::status Read(DataType* out) = 0;
  virtual absl::status Read(DataType* out, uint64_t len) = 0;
  virtual uint64_t ReadAtMost(DataType* out, uint64_t len) = 0;
  virtual std::string DebugString() const { return ""; }

 protected:
  bool closed_ = false;
};

template <typename DataType>
class Writer {
 public:
  virtual ~Writer() = default;
  virtual absl::status Write(const DataType& in) = 0;
  virtual absl::status Write(const DataType in[], uint64_t len) = 0;
  virtual void Flush() = 0;
};
}  // namespace yeeio::io
