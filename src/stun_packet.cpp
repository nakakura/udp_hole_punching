#include "stun_packet.h"

std::vector<uint8_t> STUNPacketBuilder::createBindingRequest() {
    std::vector<uint8_t> packet(STUN_HEADER_SIZE, 0x00);
    
    // Message Type: Binding Request (RFC 5389)
    packet[0] = (STUN_MESSAGE_TYPE_BINDING_REQUEST >> 8) & 0xFF;  // 上位バイト
    packet[1] = STUN_MESSAGE_TYPE_BINDING_REQUEST & 0xFF;         // 下位バイト
    
    // Message Length: 0 (属性なし、ヘッダーのみ)
    // TODO: 将来属性を追加する際は動的計算に変更する
    packet[2] = (STUN_MESSAGE_LENGTH_NO_ATTRIBUTES >> 8) & 0xFF;  // 上位バイト
    packet[3] = STUN_MESSAGE_LENGTH_NO_ATTRIBUTES & 0xFF;         // 下位バイト
    
    // Magic Cookie: 0x2112A442 (RFC 5389)
    packet[4] = (STUN_MAGIC_COOKIE >> 24) & 0xFF;  // 最上位バイト
    packet[5] = (STUN_MAGIC_COOKIE >> 16) & 0xFF;  // 上位バイト
    packet[6] = (STUN_MAGIC_COOKIE >> 8) & 0xFF;   // 下位バイト
    packet[7] = STUN_MAGIC_COOKIE & 0xFF;          // 最下位バイト
    
    // Transaction ID: 96-bit (8-19バイト目) - ランダム生成
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(0, 255);
    
    for(int i = 8; i < 20; i++) {
        packet[i] = dis(gen);
    }
    
    return packet;
}
