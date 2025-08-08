#pragma once

#include <cstdint>
#include <random>
#include <vector>

/**
 * STUNパケット生成クラス
 *
 * RFC 5389に準拠したSTUN Binding Requestパケットを生成する
 */
class StunPacketBuilder {
 private:
  // STUN Message Constants (RFC 5389)
  static const uint16_t kStunMessageTypeBindingRequest = 0x0001;
  // Magic Cookieは固定値(RFC 5389)
  static const uint32_t kStunMagicCookie = 0x2112A442;
  // TODO(nakakura): 属性追加時は動的計算
  static const uint16_t kStunMessageLengthNoAttributes = 0x0000;
  // STUNパケットのヘッダーサイズ（20バイト）
  static const size_t kStunHeaderSize = 20;

 public:
  /**
   * STUN Binding Requestパケットを生成
   *
   * @return STUNパケットのバイト列
   */
  std::vector<uint8_t> CreateBindingRequest();
};
