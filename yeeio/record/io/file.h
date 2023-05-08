/**
 * Copyright (c) 2023.04.17
 * Author: MengXii
 */

#pragma once
#include <filesystem>
#include <fstream>
#include <optional>
#include "absl/status/status.h"

#include "yeeio/record/io/base.h"

namespace yeeio::io {

class FileReader : public Reader<char> {
 public:
  explicit FileReader(const std::filesystem::path& path)
      : path_(path), stream_(path, std::ios_base::in | std::ios_base::binary) {}

  static std::unique_ptr<Reader<char>> FromPath(const std::filesystem::path& path);
  absl::Status Read(char* out) override;
  absl::Status Read(char* out, uint64_t len) override;
  uint64_t ReadAtMost(char* out, uint64_t len);

 private:
  const std::filesystem::path path_;
  std::ifstream stream_;
};

class FileWriter : public Writer<char> {
 public:
  static Writer<char>* ToPath(
      const std::filesystem::path& path,
      const std::optional<std::filesystem::path>& final_path = std::nullopt_t);

  void Write(const char& in) override { stream_.write(&in, 1); }
  void Write(const char in[], uint64_t len) override { stream_.write(in, len); }
  void Flush() override { stream_.flush(); }

 private:
  explicit FileWriter(const std::filesystem::path& path,
                      const std::optional<std::filesystem::path>& final_path)
      : path_(path),
        final_path_(final_path),
        stream_(path, std::ios_base::out | std::ios_base::binary) {
    stream_.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  }

  const std::filesystem::path path_;
  const std::optional<std::filesystem::path> final_path_;
  std::ofstream stream_;
};

}  // namespace yeeio::io