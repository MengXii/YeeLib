/**
 * Copyright (c) 2023.07.20
 * Author: MengXii
 */

#pragma once
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <cstring>
#include <memory>
#include <string>

#include "base/logging/log.h"

namespace yeelib::base::share_memory {

class ShmManager {
 public:
  ShmManager(std::string_view shm_name, size_t buf_size);
  bool ReleaseMappedMem();
  bool DestroyShm();
  virtual bool Init() = 0;
  virtual ~ShmManager() = default;

 protected:
  int OpenShm(bool create);
  void *MmapShm();

 protected:
  const std::string shm_name_;
  const size_t buf_size_;
  int fd_;
  void *addr_;
};

template <bool IsOpenShm>
class ShmService : public ShmManager {
 public:
  ShmService(std::string_view shm_name, size_t buf_size)
      : ShmManager(shm_name, buf_size) {}

  bool Init() {
    if (OpenShm(IsOpenShm) == -1) {
      return false;
    }
    if constexpr (IsOpenShm == true) {
      if (ftruncate(fd_, buf_size_) != 0) {
        int error = errno;
        LOG(INFO) << "Failed to truncate memory" << error << ", "
                  << strerror(error);
        close(fd_);
        ReleaseMappedMem();
        return false;
      }
    }
    if (!MmapShm()) {
      return false;
    }
    return true;
  }
  ~ShmService() {
    if (addr_) {
      ReleaseMappedMem();
      DestroyShm();
    }
  }
};

}  // namespace yeelib::base::share_memory
