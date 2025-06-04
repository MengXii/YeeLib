load(
    "@rules_cc//cc:defs.bzl",
    "cc_binary",
    "cc_library",
    "cc_test",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "shm_payload",
    hdrs = ["shm_payload.h"],
    deps = [
        "//base/lockfree:ring_buffer"
    ],
)

cc_library(
    name = "shm_base",
    hdrs = [
        "shm_base.h",
        "shm_receiver.h",
        "shm_sender.h",
    ],
    srcs = [
        "shm_base.cc",
    ],
    deps = [
        ":shm_payload",
    ]
)
