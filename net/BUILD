load(
    "@rules_cc//cc:defs.bzl",
    "cc_binary",
    "cc_library",
    "cc_test",
)
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_proto_grpc//cpp:defs.bzl", "cpp_proto_library")

package(default_visibility = ["//visibility:public"])

proto_library(
    name = "fd_types_proto",
    srcs = ["fd_types.proto"]
)

cpp_proto_library(
    name = "fd_types_cpp",
    protos = [":fd_types_proto"],
)

cc_library(
    name = "fd_trans_service",
    srcs = [
        "fd_trans_service.cpp",
    ],
    hdrs = [
        "fd_trans_service.h",
    ],
    deps = [
        ":fd_types_cpp",
        "//base/loop:uv_loop",
    ]
)

cc_library(
    name = "local_client",
    hdrs = ["local_client.h"],
    srcs = ["local_client.cpp"],
    deps = [
        "//base/loop:uv_loop",
        "//third_party:glog",
    ],
)
