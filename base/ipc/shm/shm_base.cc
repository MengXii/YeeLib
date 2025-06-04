/**
 * Copyright (c) 2024.08.26
 * Author: MengXii
 */
#include "shm_base.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace base::ipc::shm {

ShmBase::ShmBase(const std::string& shm_name, size_t buf_size)
    : shm_name_(shm_name), buf_size_(buf_size) {}

void* ShmBase::GetSharedAddr() { return addr_; }

bool ShmBase::DestroyShm() {
  if (shm_unlink(shm_name_.c_str()) == -1) {
    LOG(ERROR) << "shm_unlink() failed, name: " << shm_name_ << " ";
    return false;
  }
  fd_ = -1;
  addr_ = nullptr;
  return true;
}

int ShmBase::OpenShm(bool create) {
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

void* ShmBase::MmapShm() {
  addr_ = mmap(nullptr, buf_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (addr_ == MAP_FAILED) {
    LOG(ERROR) << "mmap() failed: " << errno << ", " << strerror(errno);
    close(fd_);
    addr_ = nullptr;
    return addr_;
  }

  if (close(fd_) != 0) {
    LOG(ERROR) << "close() fd failed: " << errno << ", " << strerror(errno);
    UnmapShm();
  }

  return addr_;
}

bool ShmBase::UnmapShm() {
  if (munmap(addr_, buf_size_)) {
    LOG(ERROR) << "munmap() failed: " << errno << ", " << strerror(errno);
    return false;
  }
  addr_ = nullptr;

  return true;
}

CreateShmService::CreateShmService(const std::string& shm_name, size_t buf_size)
    : ShmBase(shm_name, buf_size) {}

bool CreateShmService::Init() {
  if (OpenShm(true) == -1) {
    return false;
  }
  if (ftruncate(fd_, buf_size_) != 0) {
    LOG(ERROR) << "ftruncate() failed: " << errno << ", " << strerror(errno);
    close(fd_);
    UnmapShm();
    return false;
  }
  if (!MmapShm()) {
    return false;
  }
  return true;
}

AttachShmService::AttachShmService(const std::string& shm_name, size_t buf_size)
    : ShmBase(shm_name, buf_size) {}

bool AttachShmService::Init() {
  if (OpenShm(false) == -1) {
    return false;
  }
  if (!MmapShm()) {
    return false;
  }
  return true;
}

}  // namespace base::ipc::shm
