load(
    "@rules_cc//cc:defs.bzl",
    "cc_binary",
    "cc_library",
    "cc_test",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "uv_loop",
    hdrs = ["uv_loop.h"],
    srcs = ["uv_loop.cpp"],
    deps = [
        "//third_party:libuv"
    ],
)

cc_library(
    name = "uv_event_queue",
    hdrs = ["uv_event_queue.h"],
    srcs = [],
    deps = [
        ":uv_loop",
        "//base/lockfree:lock_free_queue",
    ],
)
