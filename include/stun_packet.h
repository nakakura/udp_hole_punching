#pragma once

#include <vector>
#include <cstdint>
#include <random>

/**
 * STUNパケット生成クラス
 *
 * RFC 5389に準拠したSTUN Binding Requestパケットを生成する
 */
class STUNPacketBuilder
{
private:
    // STUN Message Constants (RFC 5389)
    static const uint16_t STUN_MESSAGE_TYPE_BINDING_REQUEST = 0x0001;
    // Magic Cookieは固定値(RFC 5389)
    static const uint32_t STUN_MAGIC_COOKIE = 0x2112A442;
    // TODO: 属性追加時は動的計算
    static const uint16_t STUN_MESSAGE_LENGTH_NO_ATTRIBUTES = 0x0000;
    // STUNパケットのヘッダーサイズ（20バイト）
    static const size_t STUN_HEADER_SIZE = 20;

public:
    /**
     * STUN Binding Requestパケットを生成
     *
     * @return STUNパケットのバイト列
     */
    std::vector<uint8_t> createBindingRequest();
};
