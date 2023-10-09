load(
    "@rules_cc//cc:defs.bzl",
    "cc_library",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "log",
    hdrs = [
        "log.h",
    ],
    srcs = [],
    deps = [
        "//third_party:glog",
        "@com_fmt//:fmt",
    ],
)
