#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "istun_packet.h"
#include "stun_packet.h"

/**
 * STUNパケット生成のテストクラス
 */
class STUNPacketTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // テスト前の準備
  }

  void TearDown() override {
    // テスト後のクリーンアップ
  }
};

/**
 * STUN Binding Requestの最小サイズをテスト
 *
 * RFC 5389: STUNメッセージは最低20バイト（ヘッダーのみ）
 */
TEST_F(STUNPacketTest, BindingRequestHasCorrectSize) {
  STUNPacketBuilder builder;
  auto packet = builder.CreateBindingRequest();

  // 最小サイズは20バイト（ヘッダーのみ）
  EXPECT_EQ(packet.size(), 20);
}

/**
 * STUN Binding Requestのメッセージタイプをテスト
 *
 * RFC 5389: Binding Request = 0x0001
 */
TEST_F(STUNPacketTest, BindingRequestHasCorrectMessageType) {
  STUNPacketBuilder builder;
  auto packet = builder.CreateBindingRequest();

  // Message Type (0-1バイト目): Binding Request = 0x0001
  uint16_t message_type = (static_cast<uint16_t>(packet[0]) << 8U) | packet[1];
  EXPECT_EQ(message_type, 0x0001);
}

/**
 * STUN Binding RequestのMessage Typeのクラスビットをテスト
 *
 * RFC 5389: Request クラス = 0b00 (C1=0, C0=0)
 * Message Typeのビット構造: M11-M0 (Method), C1-C0 (Class)
 */
TEST_F(STUNPacketTest, BindingRequestHasCorrectClassBits) {
  STUNPacketBuilder builder;
  auto packet = builder.CreateBindingRequest();

  // Message Type (0-1バイト目)を取得
  uint16_t message_type = (static_cast<uint16_t>(packet[0]) << 8U) | packet[1];

  // クラスビット (C1, C0) を抽出
  // C1 = bit 4, C0 = bit 8 (RFC 5389のビット配置)
  uint8_t c1 = (message_type >> 4U) & 0x01U;  // bit 4
  uint8_t c0 = (message_type >> 8U) & 0x01U;  // bit 8

  // Request クラスは 0b00 (C1=0, C0=0)
  EXPECT_EQ(c1, 0) << "C1 bit should be 0 for Request class";
  EXPECT_EQ(c0, 0) << "C0 bit should be 0 for Request class";

  // 全体のクラス値も確認
  uint8_t class_value = (c1 << 1U) | c0;
  EXPECT_EQ(class_value, 0x00) << "Request class should be 0b00";
}

/**
 * STUN Binding RequestのMagic Cookieをテスト
 *
 * RFC 5389: Magic Cookie = 0x2112A442（4-7バイト目）
 */
TEST_F(STUNPacketTest, BindingRequestHasMagicCookie) {
  STUNPacketBuilder builder;
  auto packet = builder.CreateBindingRequest();

  // Magic Cookie (4-7バイト目): 0x2112A442
  uint32_t magic_cookie = (static_cast<uint32_t>(packet[4]) << 24U) |
                          (static_cast<uint32_t>(packet[5]) << 16U) |
                          (static_cast<uint32_t>(packet[6]) << 8U) | packet[7];
  EXPECT_EQ(magic_cookie, 0x2112A442);
}

/**
 * STUN Binding RequestのMessage Lengthをテスト
 *
 * RFC 5389: Message Length（2-3バイト目）= 0（属性なしの場合）
 */
TEST_F(STUNPacketTest, BindingRequestHasCorrectMessageLength) {
  STUNPacketBuilder builder;
  auto packet = builder.CreateBindingRequest();

  // Message Length (2-3バイト目): 0（ヘッダーのみの場合）
  uint16_t message_length =
      (static_cast<uint16_t>(packet[2]) << 8U) | packet[3];
  EXPECT_EQ(message_length, 0);
}

/**
 * Transaction IDの一意性をテスト
 */
TEST_F(STUNPacketTest, TransactionIdUniqueness) {
  STUNPacketBuilder builder;
  auto packet1 = builder.CreateBindingRequest();
  auto packet2 = builder.CreateBindingRequest();

  // Transaction IDが異なることを確認 (8-19バイト)
  bool different = false;
  for (int i = 8; i < 20; i++) {
    if (packet1[i] != packet2[i]) {
      different = true;
      break;
    }
  }
  EXPECT_TRUE(different) << "Transaction IDが重複しています";
}

/**
 * STUNPacketBuilderが型消去を利用して動作することを確認
 */
TEST_F(STUNPacketTest, TypeErasureWorksCorrectly) {
  std::unique_ptr<ISTUNPacketBuilder> builder =
      std::make_unique<STUNPacketBuilder>();
  auto packet = builder->CreateBindingRequest();

  // 最小サイズは20バイト（ヘッダーのみ）
  EXPECT_EQ(packet.size(), 20);
}
