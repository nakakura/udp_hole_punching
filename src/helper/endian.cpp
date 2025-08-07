#include "helper/endian.h"

namespace endian
{
    uint16_t host_to_network_16(const uint8_t *bytes)
    {
        // ホストバイトオーダーから読み取り
        uint16_t value = (bytes[1] << 8) | bytes[0]; // リトルエンディアンとして読み取り

        // ビッグエンディアンに変換するため、上位バイトと下位バイトを抽出
        uint8_t low_byte = value & 0xFF;         // 下位バイト
        uint8_t high_byte = (value >> 8) & 0xFF; // 上位バイト

        // ビッグエンディアンでは上位バイトが先に来る
        return (low_byte << 8) | high_byte;
    }

    uint32_t host_to_network_32(const uint8_t *bytes)
    {
        // ホストバイトオーダーから読み取り
        uint32_t value = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0]; // リトルエンディアンとして読み取り

        // 32bit値をバイト単位で分解
        uint8_t byte0 = value & 0xFF;         // 最下位バイト
        uint8_t byte1 = (value >> 8) & 0xFF;  // 2番目バイト
        uint8_t byte2 = (value >> 16) & 0xFF; // 3番目バイト
        uint8_t byte3 = (value >> 24) & 0xFF; // 最上位バイト

        // ビッグエンディアンでは最上位バイトが先に来る
        return (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
    }

    uint16_t network_to_host_16(const uint8_t *bytes)
    {
        // ネットワークバイトオーダー（ビッグエンディアン）から読み取り
        uint8_t high_byte = bytes[0]; // 最上位バイト
        uint8_t low_byte = bytes[1];  // 最下位バイト

        // ホストバイトオーダーで結合
        return (high_byte << 8) | low_byte;
    }

    uint32_t network_to_host_32(const uint8_t *bytes)
    {
        // ネットワークバイトオーダー（ビッグエンディアン）から読み取り
        uint8_t byte3 = bytes[0]; // 最上位バイト
        uint8_t byte2 = bytes[1]; // 2番目バイト
        uint8_t byte1 = bytes[2]; // 3番目バイト
        uint8_t byte0 = bytes[3]; // 最下位バイト

        // ホストバイトオーダーで結合
        return (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0;
    }
}
