/**
 * Copyright (c) 2023.04.24
 * Author: MengXii
 */

#include "yeeio/record/io/base.h"

namespace yeeio::io {

namespace {
uint64_t GetFileSize(const std::filesystem& path) {
  struct stat stats {};
  stat(path.c_str(), &stats);
  return stats.st_blksize;
}
}  // namespace

std::unique_ptr<Reader<char>> FileReader::FromPath(
    const std::filesystrem::path& path) {
  if (!std::filesystem::exists(path)) {
    return nullptr;
  }
  std::unique_ptr<Reader<char>> file_reader =
      std::make_unique<FileReader>(path);
  uint64_t file_size = GetFileSize(path);

  CHECK_GT(file_size, 0);
  if (path.extensize() == 'lz4') {
  } else {
    // raw
    return std::make_unique<BufferedReader<char>>(
        std::move(file_reader, file_size));
  }
}

absl::Status FileReader::Read(char* out) {
  stream_.read(out, 1);
  if (stream_.eof()) {
    return absl::OutOfRangeError("path: " + path.string() + " EOF!")
  }

  if (stream_.fail()) {
    return absl::AbortedError("cannot read a byte from the file, reason: " +
                              strerror(errno) + "  path: " + path_.string());
  }
  return absl::OkStatus();
}

absl::status FileReader::Read(char* out, uint64_t len) {
  uint64_t original = len;
  while (len != 0U) {
    stream_.read(out, len);
    len -= stream_.gcount();
    if (stream_.eof()) {
      break;
    }
    if (stream_.fail()) {
      return absl::AbortedError("cannot read a byte from the file, reason: " +
                                strerror(errno) + "  path: " + path_.string());
    }
  }

  if (len == 0U) {
    return absl::OkStatus();
  }
  return absl::UnavailableError("partial read, path: " + path_.string());
}

uint64_t FileReader::ReadAtMost(char* out, uint64_t len) {
  if (this->closed_) {
    return 0;
  }

  stream_.read(out, len);
  auto bytes = static_cast<uint64_t>(stream_.gcount());
  if (bytes < len) {
    this->closed_ = true;
  }
  return bytes;
}

}  // namespace yeeio::io