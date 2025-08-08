#pragma once

#include <uv.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

/**
 * UDPソケット管理クラス
 *
 * STUNクライアント実装でUDP通信を行うためのソケット管理機能を提供
 * RAII原則に従い、ソケットの生成・破棄を自動管理する
 * libuvを使用して非同期I/Oを実現
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
  bool IsValid() const;

  /**
   * ローカルポート番号を取得
   *
   * @return ローカルポート番号（0の場合は取得失敗）
   */
  int GetLocalPort() const;

  /**
   * UDPパケットを指定したアドレスに送信
   *
   * @param data 送信するデータ
   * @param ip 送信先IPアドレス
   * @param port 送信先ポート番号
   * @return true: 送信成功, false: 送信失敗
   */
  bool SendTo(const std::vector<uint8_t>& data, const std::string& ip,
              int port);

  /**
   * UDPパケットを受信（タイムアウト付き）
   *
   * @param data 受信したデータ
   * @param sender_ip 送信者IPアドレス
   * @param sender_port 送信者ポート番号
   * @param timeout_ms タイムアウト（ミリ秒）
   * @return true: 受信成功, false: 受信失敗またはタイムアウト
   */
  bool ReceiveFrom(std::vector<uint8_t>& data, std::string& sender_ip,
                   int& sender_port, int timeout_ms);

 private:
  std::unique_ptr<uv_loop_t> loop_;
  std::unique_ptr<uv_udp_t> udp_handle_;
  bool is_valid_;
  mutable std::mutex mutex_;  // スレッドセーフ保護用

  // 送信・受信用のコールバック関連
  struct SendContext {
    bool completed_ = false;
    bool success_ = false;
  };

  struct ReceiveContext {
    bool completed_ = false;
    bool success_ = false;
    std::vector<uint8_t> data_;
    std::string sender_ip_;
    int sender_port_ = 0;
  };

  static void OnSendComplete(uv_udp_send_t* req, int status);
  static void OnReceive(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf,
                        const struct sockaddr* addr, unsigned flags);
  static void AllocBuffer(uv_handle_t* handle, size_t suggested_size,
                          uv_buf_t* buf);
};
