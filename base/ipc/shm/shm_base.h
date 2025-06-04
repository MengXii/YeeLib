/**
 * Copyright (c) 2024.08.20
 * Author: MengXii
 */

#include "shm_payload.h"

namespace base::ipc::shm {

class ShmBase {
 public:
  ShmBase(const std::string &shm_name, size_t buf_size);

  void *GetSharedAddr();
  virtual bool Init() = 0;
  virtual ~ShmBase() = default;

 protected:
  int OpenShm(bool create);
  void *MmapShm();
  bool UnmapShm();
  bool DestroyShm();

 protected:
  const std::string shm_name_;
  const size_t buf_size_;
  int fd_;
  void *addr_;
};

class CreateShmService : public ShmBase {
 public:
  CreateShmService(const std::string &shm_name, size_t buf_size);
  bool Init() override;
  ~CreateShmService() {
    if (addr_) {
      UnmapShm();
      DestroyShm();
    }
  }
};

class AttachShmService : public ShmBase {
 public:
  AttachShmService(const std::string &shm_name, size_t buf_size);
  bool Init() override;
  ~AttachShmService() {
    if (addr_) {
      UnmapShm();
      DestroyShm();
    }
  }
};

}  // namespace base::ipc::shm
