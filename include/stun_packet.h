#pragma once

#include <vector>
#include <cstdint>
#include <random>

/**
 * STUNメッセージヘッダー構造体
 * 
 * RFC 5389 STUN Message Header Structure
 */
struct STUNMessageHeader {
    uint16_t message_type;
    uint16_t message_length;
    uint32_t magic_cookie;
    uint8_t transaction_id[12];  // 96-bit Transaction ID
    
    /**
     * デフォルトコンストラクタ
     */
    STUNMessageHeader();
    
    /**
     * パラメータ付きコンストラクタ
     * 
     * @param msg_type メッセージタイプ
     * @param msg_length メッセージ長
     * @param cookie Magic Cookie
     */
    STUNMessageHeader(uint16_t msg_type, uint16_t msg_length, uint32_t cookie);
    
    /**
     * ヘッダーをバイト列にシリアライズ
     * 
     * @return STUNヘッダーのバイト列
     */
    std::vector<uint8_t> serialize() const;
};

/**
 * STUNパケット生成クラス
 * 
 * RFC 5389に準拠したSTUN Binding Requestパケットを生成する
 */
class STUNPacketBuilder {
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
