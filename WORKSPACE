#
# Copyright (c) 2023.03.06
# Author: MengXii
#

workspace(name = "YeeLib")

# repo
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

http_archive(
    name = "bazel_skylib",
    sha256 = "97e70364e9249702246c0e9444bccdc4b847bed1eb03c5a3ece4f83dfe6abc44",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
    ],
)
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
bazel_skylib_workspace()

# ---------------------------------------------------------------------------- #
#                                      C++                                     #
# ---------------------------------------------------------------------------- #
http_archive(
    name = "rules_cc",
    sha256 = "e75dfb05bc1e89ebbb6696cadb5e455833690009310d9dc5512151c5adb0e4e3",
    strip_prefix = "rules_cc-cfe68f6bc79dea602f2f6a767797f94a5904997f",
    urls = [
        "https://github.com/bazelbuild/rules_cc/archive/cfe68f6bc79dea602f2f6a767797f94a5904997f.zip",
    ],
)

# ---------------------------------------------------------------------------- #
#                                      lz4                                     #
# ---------------------------------------------------------------------------- #
new_git_repository(
    name = "com_lz4_lib",
    tag = "v1.9.4",
    remote = "git@github.com:lz4/lz4.git",
    build_file = "//third_party:lz4lib.BUILD",
)

# ---------------------------------------------------------------------------- #
#                                      absl                                    #
# ---------------------------------------------------------------------------- #
http_archive(
    name = "com_absl",
    sha256 = "4208129b49006089ba1d6710845a45e31c59b0ab6bff9e5788a87f55c5abd602",
    strip_prefix = "abseil-cpp-20220623.0",
    urls = [
        "https://github.com/abseil/abseil-cpp/archive/refs/tags/20220623.0.tar.gz",
    ],
)

# ---------------------------------------------------------------------------- #
#                                   benchmark                                  #
# ---------------------------------------------------------------------------- #

http_archive(
    name = "com_google_benchmark",
    urls = ["https://github.com/google/benchmark/archive/refs/tags/v1.6.0.tar.gz"],
    sha256 = "1f71c72ce08d2c1310011ea6436b31e39ccab8c2db94186d26657d41747c85d6",
    strip_prefix = "benchmark-1.6.0",    
)

# ---------------------------------------------------------------------------- #
#                                   gsl-lite                                   #
# ---------------------------------------------------------------------------- #
http_archive(
    name = "com_github_gsl-lite",
    build_file = "//third_party:gsl-lite.BUILD",
    sha256 = "65af4ec8a1050dac4f1ca4622881bb02a9c3978a9baec289fb56e25412d6cac7",
    strip_prefix = "gsl-lite-0.40.0",
    urls = [
        "https://github.com/gsl-lite/gsl-lite/archive/refs/tags/v0.40.0.tar.gz",
    ],
)

# ---------------------------------------------------------------------------- #
#                                  fmt                                         #
# ---------------------------------------------------------------------------- #
http_archive(
    name = "com_fmt",
    build_file = "//third_party:fmt:fmt.BUILD",
    sha256 = "65af4ec8a1050dac4f1ca4622881bb02a9c3978a9baec289fb56e25412d6cac7",
    strip_prefix = "fmt-9.1.0",
    urls = [
        "https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.tar.gz",
    ],
)
