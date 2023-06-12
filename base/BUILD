load(
    "@rules_cc//cc:defs.bzl",
    "cc_library",
    "cc_binary",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cpp_feature",
    hdrs = [
        "cpp_feature.h",
    ],
    srcs = [],
    deps = [],
)

cc_library(
    name = "event_loop",
    hdrs = [
        "event_loop.h"
    ],
    srcs = [
        "event_loop.cpp",
    ],
    deps = [
        "//third_party:libevent",
        "//third_party:glog",
    ],
)

cc_library(
    name = "event_queue",
    hdrs = [
        "event_queue.h",
        "event_queue_impl.h",
    ],
    srcs = [],
    deps = [
        "//third_party:libevent",
        "//third_party:glog",
        "@com_github_gsl-lite//:gsl-lite",
    ],
)
