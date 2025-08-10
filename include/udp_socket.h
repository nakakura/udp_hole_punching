#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <uv.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "iudp_socket.h"

/**
 * UDPソケット管理クラス
 *
 * STUNクライアント実装でUDP通信を行うためのソケット管理機能を提供
 * RAII原則に従い、ソケットの生成・破棄を自動管理する
 * libuvを使用して非同期I/Oを実現
 */
class UDPSocket : public IUdpSocket {
 public:
  /**
   * UDPソケットを作成
   */
  UDPSocket();

  /**
   * デストラクタ - ソケットを自動クローズ
   */
  ~UDPSocket() override;

  // Rule of Five: コピー・ムーブを明示的に削除
  // UDPSocketはlibuvリソースを管理するRAIIクラスのため、
  // コピーやムーブは危険（重複解放やリソース競合の可能性）
  UDPSocket(const UDPSocket&) = delete;
  auto operator=(const UDPSocket&) -> UDPSocket& = delete;
  UDPSocket(UDPSocket&&) = delete;
  auto operator=(UDPSocket&&) -> UDPSocket& = delete;

  // Override IUdpSocket methods with improvements
  [[nodiscard]] auto IsValid() const -> bool override;
  [[nodiscard]] auto GetLocalPort() const -> int override;
  auto SendTo(const std::vector<uint8_t>& data, const std::string& ip,
              int port) -> bool override;
  auto ReceiveFrom(std::vector<uint8_t>& data, std::string& sender_ip,
                   int& sender_port, int timeout_ms) -> bool override;

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
  static void OnReceive(
      uv_udp_t* handle, ssize_t nread,
      const uv_buf_t* buf,  // NOLINT(misc-include-cleaner) libuv公式API
      const struct sockaddr* addr, unsigned flags);
  static void AllocBuffer(
      uv_handle_t* handle, size_t suggested_size,
      uv_buf_t* buf);  // NOLINT(misc-include-cleaner) libuv公式API
};

#endif  // UDP_SOCKET_H
