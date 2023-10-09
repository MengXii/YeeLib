load(
    "@rules_cc//cc:defs.bzl",
    "cc_library",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "normal_tcp_client",
    hdrs = [
        "normal_tcp_client.h",
        "normal_tcp_connection.h"
    ],
    srcs = [
        "normal_tcp_client.cpp",
        "normal_tcp_connection.cpp",
    ],
    deps = [
        "//network:client",
        "//third_party:libevent",
        "//third_party:glog",
        "@com_absl//absl/status",
    ]
)