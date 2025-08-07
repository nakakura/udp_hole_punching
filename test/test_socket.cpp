#include <gtest/gtest.h>
#include "socket.h"

/**
 * UDPSocketクラスのテスト
 */
class UDPSocketTest : public ::testing::Test
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
 * UDPソケットが正常に作成できることを確認
 */
TEST_F(UDPSocketTest, CanCreateUDPSocket)
{
    UDPSocket socket;

    // ソケットが正常に作成されていることを確認
    EXPECT_TRUE(socket.isValid());

    // ファイルディスクリプタが有効であることを確認
    EXPECT_GE(socket.getFd(), 0);
}

/**
 * ソケットファイルディスクリプタが正しく管理されることを確認
 */
TEST_F(UDPSocketTest, FileDescriptorManagement)
{
    UDPSocket socket;

    // 有効なファイルディスクリプタが取得できることを確認
    int fd = socket.getFd();
    EXPECT_GE(fd, 0);

    // 同じインスタンスから同じFDが取得できることを確認
    EXPECT_EQ(fd, socket.getFd());
}
