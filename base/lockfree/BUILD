load(
    "@rules_cc//cc:defs.bzl",
    "cc_binary",
    "cc_library",
    "cc_test",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "lock_free_queue",
    hdrs = ["lock_free_queue.h"],
    deps = [
    ],
)

cc_library(
    name = "ring_buffer",
    hdrs = ["ring_buffer.h"],
    deps = [
        "//third_party:glog"
    ],
)

cc_test(
    name = "ring_buffer_test",
    srcs = [
        "ring_buffer_test.cc",
    ],
    deps = [
        ":ring_buffer",
        "//third_party:glog",
        "//third_party:gtest",  # not gtest_main        
    ]
)
