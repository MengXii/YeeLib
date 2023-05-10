load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_binary")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "bench",
    srcs = [
        "benchmark.cpp",
    ],
    deps = [
        "@com_google_benchmark//:benchmark"
    ],
)