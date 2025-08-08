#pragma once

#include <string>

/**
 * UDPソケット管理クラス
 *
 * STUNクライアント実装でUDP通信を行うためのソケット管理機能を提供
 * RAII原則に従い、ソケットの生成・破棄を自動管理する
 */
class UDPSocket {
 public:
  /**
   * UDPソケットを作成
   */
  UDPSocket();

  /**
   * デストラクタ - ソケットを自動クローズ
   */
  ~UDPSocket();

  /**
   * ソケットが正常に作成されているかチェック
   *
   * @return true: ソケット作成済み, false: 作成失敗または未作成
   */
  bool isValid() const;

  /**
   * ソケットファイルディスクリプタを取得
   *
   * @return ソケットファイルディスクリプタ（-1の場合は無効）
   */
  int getFd() const;

 private:
  int socket_fd_;
};
