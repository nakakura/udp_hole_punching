#include "helper/endian.h"

#include <cstdint>

namespace endian {
auto HostToNetwork16(const uint8_t* bytes) -> uint16_t {
  // ホストバイトオーダーから読み取り
  const uint16_t value =
      (static_cast<uint16_t>(bytes[1]) << 8U) |
      static_cast<uint16_t>(bytes[0]);  // リトルエンディアンとして読み取り

  // ビッグエンディアンに変換するため、上位バイトと下位バイトを抽出
  const uint8_t low_byte = value & 0xFFU;           // 下位バイト
  const uint8_t high_byte = (value >> 8U) & 0xFFU;  // 上位バイト

  // ビッグエンディアンでは上位バイトが先に来る
  return (static_cast<uint16_t>(low_byte) << 8U) |
         static_cast<uint16_t>(high_byte);
}

auto HostToNetwork32(const uint8_t* bytes) -> uint32_t {
  // ホストバイトオーダーから読み取り
  const uint32_t value =
      (static_cast<uint32_t>(bytes[3]) << 24U) |
      (static_cast<uint32_t>(bytes[2]) << 16U) |
      (static_cast<uint32_t>(bytes[1]) << 8U) |
      static_cast<uint32_t>(bytes[0]);  // リトルエンディアンとして読み取り

  // 32bit値をバイト単位で分解
  const uint8_t byte0 = value & 0xFFU;           // 最下位バイト
  const uint8_t byte1 = (value >> 8U) & 0xFFU;   // 2番目バイト
  const uint8_t byte2 = (value >> 16U) & 0xFFU;  // 3番目バイト
  const uint8_t byte3 = (value >> 24U) & 0xFFU;  // 最上位バイト

  // ビッグエンディアンでは最上位バイトが先に来る
  return (static_cast<uint32_t>(byte0) << 24U) |
         (static_cast<uint32_t>(byte1) << 16U) |
         (static_cast<uint32_t>(byte2) << 8U) | static_cast<uint32_t>(byte3);
}

auto NetworkToHost16(const uint8_t* bytes) -> uint16_t {
  // ネットワークバイトオーダー（ビッグエンディアン）から読み取り
  const uint8_t high_byte = bytes[0];  // 最上位バイト
  const uint8_t low_byte = bytes[1];   // 最下位バイト

  // ホストバイトオーダーで結合
  return (static_cast<uint16_t>(high_byte) << 8U) |
         static_cast<uint16_t>(low_byte);
}

auto NetworkToHost32(const uint8_t* bytes) -> uint32_t {
  // ネットワークバイトオーダー（ビッグエンディアン）から読み取り
  const uint8_t byte3 = bytes[0];  // 最上位バイト
  const uint8_t byte2 = bytes[1];  // 2番目バイト
  const uint8_t byte1 = bytes[2];  // 3番目バイト
  const uint8_t byte0 = bytes[3];  // 最下位バイト

  // ホストバイトオーダーで結合
  return (static_cast<uint32_t>(byte3) << 24U) |
         (static_cast<uint32_t>(byte2) << 16U) |
         (static_cast<uint32_t>(byte1) << 8U) | static_cast<uint32_t>(byte0);
}
}  // namespace endian
