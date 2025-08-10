#ifndef IUDP_SOCKET_H
#define IUDP_SOCKET_H

#include <cstdint>
#include <string>
#include <vector>

class IUdpSocket {
 public:
  virtual ~IUdpSocket() = default;

  // データを送信する
  virtual bool SendTo(const std::vector<uint8_t>& data,
                      const std::string& address, int port) = 0;

  // データを受信する
  virtual bool ReceiveFrom(std::vector<uint8_t>& buffer, std::string& address,
                           int& port, int timeout_ms) = 0;

  // ソケットが正常に作成されているかチェック
  virtual bool IsValid() const = 0;

  // ローカルポート番号を取得
  virtual int GetLocalPort() const = 0;
};

#endif  // IUDP_SOCKET_H
