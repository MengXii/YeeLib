/**
 * Copyright (c) 2024.08.13
 * Author: MengXii
 */

#include "base/net/fd_trans_service.h"

#include <thread>

#include "base/loop/uv_loop.h"
#include "base/net/local_client.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace base::net::testing {
using namespace base::loop;
TEST(FD_TRANS_SERVICE_TEST, data) {
  UvEpollLoop* loop = new UvEpollLoop();
  FdTransService fd_service(loop);

  std::string sock = "/tmp/test.sock";

  CHECK(fd_service.BindDomainSocket(sock));

  LocalClient lc(loop, sock, "local_client", 1);
  lc.Connect();

  auto thread_functor = [loop]() {
    LOG(INFO) << "Thread started";
    loop->Run();
    LOG(INFO) << "Thread exiting";
  };
  std::thread t1(thread_functor);
  sleep(1);

  FdMsg fd_msg;
  fd_msg.set_type(FdMsg::FD_REGISTER);
  fd_msg.set_service_name(lc.service_name());
  fd_msg.add_fds(1);

  char data[2024];
  int len = fd_msg.ByteSizeLong();
  fd_msg.SerializeToArray(data, len);

  CHECK(lc.SendData(data, len));

  fd_msg.set_service_name("hhh");
  len = fd_msg.ByteSizeLong();
  fd_msg.SerializeToArray(data, len);
  CHECK(lc.SendData(data, len));
  sleep(1);
  loop->Stop();
  t1.join();
}

}  // namespace base::net::testing

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}