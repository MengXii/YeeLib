load(
    "@rules_cc//cc:defs.bzl",
    "cc_library",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "shm_manager",
    hdrs = [
        "shm_manager.h",
    ],
    srcs = [
        "shm_manager.cpp",
    ],
    deps = [
        "//base/logging:log",
    ],
)
