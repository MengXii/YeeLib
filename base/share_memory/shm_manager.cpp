/**
 * Copyright (c) 2023.07.20
 * Author: MengXii
 */

#include "shm_manager.h"

namespace yeelib::base::share_memory {

ShmManager::ShmManager(std::string_view shm_name, size_t buf_size)
    : shm_name_(shm_name), buf_size_(buf_size), fd_(-1), addr_(nullptr) {}

bool ShmManager::ReleaseMappedMem() {
  if (munmap(addr_, buf_size_)) {
    int error = errno;
    LOG(ERROR) << "munmap failed: " << error << ", " << strerror(error);
    return false;
  }
  addr_ = nullptr;

  return true;
}

bool ShmManager::DestroyShm() {
  if (shm_unlink(shm_name_.c_str())) {
    int error = errno;
    if (error != ENOENT) {
      LOG(ERROR) << "shm_unlink failed: " << error << ", " << strerror(error);
    }
    return false;
  }
  fd_ = -1;
  addr_ = nullptr;

  return true;
}

int ShmManager::OpenShm(bool create) {
  int flag = O_RDWR;
  if (create) {
    DestroyShm();
    flag |= O_CREAT | O_EXCL;
  }

  fd_ = shm_open(shm_name_.c_str(), flag, S_IRUSR | S_IWUSR);
  if (fd_ == -1) {
    LOG(ERROR) << "Failed to create shared memory: " << errno << ", "
               << strerror(errno);
  }
  return fd_;
}

void *ShmManager::MmapShm() {
  CHECK(fd_ != -1);
  addr_ = mmap(nullptr, buf_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (addr_ == MAP_FAILED) {
    int error = errno;
    LOG(ERROR) << "mmap failed: " << error << ", " << strerror(error);
    close(fd_);
    addr_ = nullptr;
    return addr_;
  }

  if (close(fd_) != 0) {
    int error = errno;
    LOG(ERROR) << "close failed: " << error << ", " << strerror(error);
    ReleaseMappedMem();
  }

  return addr_;
}

}  // namespace yeelib::base::share_memory
