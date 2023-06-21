/**
 * Copyright (c) 2023.06.21
 * Author: MengXii
 */

// Faster implementation:
// https://kholdstare.github.io/technical/2020/05/26/faster-integer-parsing.html

#pragma once

#include <cstdint>

struct NormalVersionTag {};
struct PartialVersionTag {};

template <typename Char>
inline bool IsDigit(Char c) {
  return c >= '0' && c <= '9';
}

template <int N>
inline bool IsDigit(const char* string) {
  static_assert(N > 1, "N must be greater than 1");
  return IsDigit(string[0]) && IsDigit<N - 1>(string + 1);
}

template <>
inline bool IsDigit<1>(const char* string) {
  return IsDigit(string[0]);
}

template <typename Char>
inline bool IsHexDigit(Char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
         (c >= 'a' && c <= 'f');
}

template <int N>
inline bool IsHexDigit(const char* str) {
  static_assert(N > 1, "N must be greater than 1");
  return IsHexDigit(str[0]) && IsHexDigit<N - 1>(str + 1);
}

template <>
inline bool IsHexDigit<1>(const char* str) {
  return IsHexDigit(str[0]);
}

// Treat as a c-string.
template <int N>
inline bool IsHexDigit(const char (&str)[N]) {
  // do not check last char.
  return IsHexDigit<N - 1>(reinterpret_cast<const char*>(&str));
}

template <typename Char>
inline Char HexDigitToInt(Char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

template <int N>
struct TagDispatchTrait {
  using Tag = NormalVersionTag;
};

template <>
struct TagDispatchTrait<1> {
  using Tag = PartialVersionTag;
};

template <typename IntType>
inline IntType DigitToInt(const char in) {
  return in - '0';
}

template <typename IntType, int N>
IntType ParseIntImpl(const char* str, IntType in, NormalVersionTag) {
  return ParseIntImpl<IntType, N - 1>(str + 1,
                                      in * 10 + DigitToInt<IntType>(str[0]),
                                      typename TagDispatchTrait<N - 1>::Tag{});
}

template <typename IntType, int N>
IntType ParseIntImpl(const char* str, IntType in, PartialVersionTag) {
  return in * 10 + DigitToInt<IntType>(str[0]);
}

template <typename IntType, int N>
bool ParseInt(const char* str, IntType* out) {
  if (IsDigit<N>(str)) {
    *out =
        ParseIntImpl<IntType, N>(str, 0, typename TagDispatchTrait<N>::Tag{});
    return true;
  } else {
    return false;
  }
}

template <typename IntType, int N>
inline bool ParseInt(const char (&str)[N], IntType* out) {
  return ParseInt<IntType, N - 1>(reinterpret_cast<const char*>(&str), out);
}

template <typename IntType, int N>
IntType ParseHexIntImpl(const char* str, IntType in, NormalVersionTag) {
  return ParseHexIntImpl<IntType, N - 1>(
      str + 1, in * 16 + HexDigitToInt<IntType>(str[0]),
      typename TagDispatchTrait<N - 1>::Tag{});
}

template <typename IntType, int N>
IntType ParseHexIntImpl(const char* str, IntType in, PartialVersionTag) {
  return in * 16 + HexDigitToInt<IntType>(str[0]);
}

template <typename IntType, int N>
bool ParseHexInt(const char* str, IntType* out) {
  if (IsHexDigit<N>(str)) {
    *out = ParseHexIntImpl<IntType, N>(str, 0,
                                       typename TagDispatchTrait<N>::Tag{});
    return true;
  } else {
    return false;
  }
}

template <typename IntType, int N>
inline bool ParseHexInt(const char (&str)[N], IntType* out) {
  return ParseHexInt<IntType, N - 1>(reinterpret_cast<const char*>(&str), out);
}
