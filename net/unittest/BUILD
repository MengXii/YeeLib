load("@rules_cc//cc:defs.bzl", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_test(
    name = "fd_trans_service_test",
    srcs = ["fd_trans_service_test.cpp"],
    deps = [
        "//base/loop:uv_loop",
        "//base/net:fd_trans_service",
        "//base/net:local_client",
        "//third_party:glog",
        "//third_party:gtest",  # not gtest_main
        "@com_github_gsl-lite//:gsl-lite",
    ]
)
