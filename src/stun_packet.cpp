#include "stun_packet.h"

#include <string.h>

#include "helper/endian.h"

std::vector<uint8_t> StunPacketBuilder::CreateBindingRequest() {
  std::vector<uint8_t> packet(kStunHeaderSize, 0x00);

  // Message Type: Binding Request (RFC 5389) - ビッグエンディアンで直接設定
  packet[0] = (kStunMessageTypeBindingRequest >> 8) & 0xFF;  // 上位バイト
  packet[1] = kStunMessageTypeBindingRequest & 0xFF;         // 下位バイト

  // Message Length: 0 (属性なし、ヘッダーのみ) - ビッグエンディアンで直接設定
  packet[2] = (kStunMessageLengthNoAttributes >> 8) & 0xFF;  // 上位バイト
  packet[3] = kStunMessageLengthNoAttributes & 0xFF;         // 下位バイト

  // Magic Cookie: 0x2112A442 (RFC 5389) - ビッグエンディアンで直接設定
  packet[4] = (kStunMagicCookie >> 24) & 0xFF;  // 最上位バイト
  packet[5] = (kStunMagicCookie >> 16) & 0xFF;  // 上位バイト
  packet[6] = (kStunMagicCookie >> 8) & 0xFF;   // 下位バイト
  packet[7] = kStunMagicCookie & 0xFF;          // 最下位バイト

  // Transaction ID: 96-bit (8-19バイト目) - 乱数なのでエンディアン変換不要
  static thread_local std::mt19937 gen(std::random_device{}());

  // 32bit単位で直接パケットに書き込み
  // 乱数なのでバイト順序は気にせず最速で書き込む
  // STUN_HEADER_SIZEは定数のため、長さチェックも省く
  *reinterpret_cast<uint32_t*>(&packet[8]) = gen();
  *reinterpret_cast<uint32_t*>(&packet[12]) = gen();
  *reinterpret_cast<uint32_t*>(&packet[16]) = gen();

  return packet;
}
