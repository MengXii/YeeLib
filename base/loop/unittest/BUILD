load("@rules_cc//cc:defs.bzl", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_test(
    name = "uv_event_queue_test",
    srcs = ["uv_event_queue_test.cpp"],
    deps = [
        "//base/loop:uv_loop",
        "//base/loop:uv_event_queue",
        "//third_party:glog",
        "//third_party:gtest",  # not gtest_main
        "@com_github_gsl-lite//:gsl-lite",
    ]
)
