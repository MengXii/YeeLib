load(
    "@rules_cc//cc:defs.bzl",
    "cc_test",
)

package(default_visibility = ["//visibility:public"])

cc_test(
    name = "event_queue_unittest",
    srcs = [
        "event_queue_unittest.cpp",
    ],
    deps = [
        "//base:event_loop",
        "//base:event_queue",
        "//third_party:gtest",
        "//third_party:gtest_main",  
    ]
)

cc_test(
    name = "ring_buffer_unittest",
    srcs = [
        "ring_buffer_unittest.cpp",
    ],
    deps = [
        "//base/lockfree:ring_buffer",
        "//third_party:gtest",
        "//third_party:gtest_main",
    ]
)