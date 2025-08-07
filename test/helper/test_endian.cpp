#include <gtest/gtest.h>
#include <vector>
#include <cstdint>

#include "helper/endian.h"

/**
 * エンディアン変換関数のテストクラス
 */
class EndianTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // テスト前の準備
    }

    void TearDown() override
    {
        // テスト後のクリーンアップ
    }
};

/**
 * 16bit値のネットワークバイトオーダー変換をテスト
 *
 * htons()相当の機能をテスト
 */
TEST_F(EndianTest, HostToNetwork16)
{
    // 0x0001をホストバイトオーダーのバイト配列として準備
    std::vector<uint8_t> host_bytes = {0x01, 0x00}; // リトルエンディアン形式
    uint16_t network_value = endian::host_to_network_16(host_bytes.data());

    // ビッグエンディアンで格納されることを確認
    EXPECT_EQ((network_value >> 8) & 0xFF, 0x01); // 上位バイト
    EXPECT_EQ(network_value & 0xFF, 0x00);        // 下位バイト
}

/**
 * 32bit値のネットワークバイトオーダー変換をテスト
 *
 * htonl()相当の機能をテスト
 */
TEST_F(EndianTest, HostToNetwork32)
{
    // Magic Cookie (0x2112A442)をホストバイトオーダーのバイト配列として準備
    std::vector<uint8_t> host_bytes = {0x42, 0xA4, 0x12, 0x21}; // リトルエンディアン形式
    uint32_t network_value = endian::host_to_network_32(host_bytes.data());

    // ネットワークバイトオーダー（ビッグエンディアン）では
    // バイト順が逆転される（リトルエンディアンシステムの場合）
    EXPECT_EQ((network_value >> 24) & 0xFF, 0x42); // 最上位バイト
    EXPECT_EQ((network_value >> 16) & 0xFF, 0xA4); // 2番目バイト
    EXPECT_EQ((network_value >> 8) & 0xFF, 0x12);  // 3番目バイト
    EXPECT_EQ(network_value & 0xFF, 0x21);         // 最下位バイト
}

/**
 * バイト配列から16bit値を読み取るテスト
 *
 * ntohs()相当の機能をテスト
 */
TEST_F(EndianTest, NetworkToHost16)
{
    // ネットワークバイトオーダーのバイト配列
    std::vector<uint8_t> bytes = {0x00, 0x01}; // 0x0001のビッグエンディアン

    uint16_t host_value = endian::network_to_host_16(bytes.data());

    EXPECT_EQ(host_value, 0x0001);
}

/**
 * バイト配列から32bit値を読み取るテスト
 *
 * ntohl()相当の機能をテスト
 */
TEST_F(EndianTest, NetworkToHost32)
{
    // ネットワークバイトオーダーのバイト配列 (Magic Cookie)
    std::vector<uint8_t> bytes = {0x21, 0x12, 0xA4, 0x42};

    uint32_t host_value = endian::network_to_host_32(bytes.data());

    EXPECT_EQ(host_value, 0x2112A442);
}

/**
 * エンディアンの変換対称性をテスト
 *
 * host→network→hostの変換で元の値に戻ることを確認
 */
TEST_F(EndianTest, ConversionSymmetry16bit)
{
    uint16_t original = 0x1234;

    // 元の値をホストバイトオーダーのバイト配列として準備
    std::vector<uint8_t> host_bytes = {
        static_cast<uint8_t>(original & 0xFF),
        static_cast<uint8_t>((original >> 8) & 0xFF)}; // リトルエンディアン形式

    // host→network→hostで元に戻ることを確認
    uint16_t network = endian::host_to_network_16(host_bytes.data());

    // networkの値をバイト配列として解釈（ネットワークバイトオーダー）
    uint8_t *bytes = reinterpret_cast<uint8_t *>(&network);

    uint16_t restored = endian::network_to_host_16(bytes);

    EXPECT_EQ(original, restored);
}

/**
 * エンディアンの変換対称性をテスト（32bit）
 *
 * host→network→hostの変換で元の値に戻ることを確認
 */
TEST_F(EndianTest, ConversionSymmetry32bit)
{
    uint32_t original = 0x12345678;

    // 元の値をホストバイトオーダーのバイト配列として準備
    std::vector<uint8_t> host_bytes = {
        static_cast<uint8_t>(original & 0xFF),
        static_cast<uint8_t>((original >> 8) & 0xFF),
        static_cast<uint8_t>((original >> 16) & 0xFF),
        static_cast<uint8_t>((original >> 24) & 0xFF)}; // リトルエンディアン形式

    // host→network→hostで元に戻ることを確認
    uint32_t network = endian::host_to_network_32(host_bytes.data());

    // networkの値をバイト配列として解釈（ネットワークバイトオーダー）
    uint8_t *bytes = reinterpret_cast<uint8_t *>(&network);

    uint32_t restored = endian::network_to_host_32(bytes);

    EXPECT_EQ(original, restored);
}
