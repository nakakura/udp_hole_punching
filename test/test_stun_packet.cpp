#include <gtest/gtest.h>
#include <vector>
#include <cstdint>

// TODO: 実装後にincludeする
// #include "stun_packet.h"

/**
 * STUNパケット生成のテストクラス
 */
class STUNPacketTest : public ::testing::Test
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
 * STUN Binding Requestパケットの基本構造をテスト
 *
 * RFC 5389準拠:
 * - Message Type: 0x0001 (Binding Request)
 * - Magic Cookie: 0x2112A442
 * - Transaction ID: 96 bits (12 bytes)
 */
TEST_F(STUNPacketTest, CreateBindingRequest)
{
    // TODO: STUNパケット生成クラスの実装後に実装
    GTEST_SKIP() << "STUNパケット生成クラスが未実装";

    /*
    // 期待する実装:
    STUNPacketBuilder builder;
    auto packet = builder.createBindingRequest();

    // パケットサイズの確認 (最小20バイト)
    ASSERT_GE(packet.size(), 20);

    // Message Type (Binding Request = 0x0001)
    uint16_t message_type = (packet[0] << 8) | packet[1];
    EXPECT_EQ(message_type, 0x0001);

    // Magic Cookie (0x2112A442)
    uint32_t magic_cookie = (packet[4] << 24) | (packet[5] << 16) |
                           (packet[6] << 8) | packet[7];
    EXPECT_EQ(magic_cookie, 0x2112A442);

    // Transaction IDが存在することを確認 (8-19バイト)
    bool has_transaction_id = true;
    for(int i = 8; i < 20; i++) {
        // 全て0でないことを確認（適切なランダム値が設定されている）
        if(packet[i] != 0) {
            has_transaction_id = true;
            break;
        }
    }
    EXPECT_TRUE(has_transaction_id);
    */
}

/**
 * STUNメッセージヘッダーの構造をテスト
 */
TEST_F(STUNPacketTest, MessageHeaderStructure)
{
    GTEST_SKIP() << "STUNメッセージヘッダー構造体が未実装";

    /*
    // 期待する実装:
    STUNMessageHeader header;
    header.message_type = 0x0001;  // Binding Request
    header.message_length = 0;     // 属性なし
    header.magic_cookie = 0x2112A442;
    // header.transaction_id は自動生成

    auto packet = header.serialize();
    EXPECT_EQ(packet.size(), 20);  // ヘッダーのみ
    */
}

/**
 * Transaction IDの一意性をテスト
 */
TEST_F(STUNPacketTest, TransactionIdUniqueness)
{
    GTEST_SKIP() << "Transaction ID生成機能が未実装";

    /*
    // 期待する実装:
    STUNPacketBuilder builder;
    auto packet1 = builder.createBindingRequest();
    auto packet2 = builder.createBindingRequest();

    // Transaction IDが異なることを確認 (8-19バイト)
    bool different = false;
    for(int i = 8; i < 20; i++) {
        if(packet1[i] != packet2[i]) {
            different = true;
            break;
        }
    }
    EXPECT_TRUE(different) << "Transaction IDが重複しています";
    */
}
