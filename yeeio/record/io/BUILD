load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_binary")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "lz4",
    hdrs = [
        "lz4.h",
    ],
    srcs = [
    ],
    deps = [
        "@com_lz4_lib//:lz4_lib",
        "@com_absl//absl/status",
    ],
)

cc_binary(
    name = "app",
    srcs = [
        "app.cpp",
    ],
    deps = [
        "@com_lz4_lib//:lz4_lib",
        "@com_absl//absl/status",
    ],
)