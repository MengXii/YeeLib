load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "gtest",
    linkopts = ["-lgtest"],
)

cc_library(
    name = "gtest_main",
    linkopts = ["-lgtest_main"],
)

cc_library(
    name = "gflags",
    linkopts = ["-lgflags"],
)

cc_library(
    name = "glog",
    linkopts = ["-lglog"],
    deps = [":gflags"],
)

cc_library(
   name = "libevent",
   linkopts = [" -levent"],
)
