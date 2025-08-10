#ifndef ISTUN_PACKET_H
#define ISTUN_PACKET_H

#include <cstdint>
#include <vector>

/**
 * STUNパケット生成の抽象インターフェース
 *
 * 依存性注入を可能にし、テスト容易性と拡張性を向上させる
 */
class ISTUNPacketBuilder {
 public:
  virtual ~ISTUNPacketBuilder() = default;

  /**
   * STUN Binding Requestパケットを生成
   *
   * @return STUNパケットのバイト列
   */
  virtual auto CreateBindingRequest() -> std::vector<uint8_t> = 0;
};

#endif  // ISTUN_PACKET_H
