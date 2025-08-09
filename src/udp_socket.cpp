#include "udp_socket.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <uv.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

UDPSocket::UDPSocket()
    : loop_(std::make_unique<uv_loop_t>()),
      udp_handle_(std::make_unique<uv_udp_t>()),
      is_valid_(false) {
  // libuvイベントループを初期化
  if (uv_loop_init(loop_.get()) != 0) {
    return;
  }

  // UDPハンドルを初期化
  if (uv_udp_init(loop_.get(), udp_handle_.get()) != 0) {
    uv_loop_close(loop_.get());
    return;
  }

  // UDPソケットを任意のポートにバインド（ファイルディスクリプタ取得のため）
  struct sockaddr_in bind_addr {};
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_addr.s_addr = INADDR_ANY;
  bind_addr.sin_port = 0;  // OSが自動でポート選択

  if (uv_udp_bind(
          udp_handle_.get(),
          reinterpret_cast<const struct sockaddr*>(
              &bind_addr),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                            // libuv API requirement
          0) != 0) {
    uv_close(reinterpret_cast<uv_handle_t*>(udp_handle_.get()),
             nullptr);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                        // libuv API requirement
    uv_loop_close(loop_.get());
    return;
  }

  is_valid_ = true;
}

UDPSocket::~UDPSocket() {
  if (is_valid_) {
    // UDPハンドルをクローズ
    if (udp_handle_) {
      uv_close(reinterpret_cast<uv_handle_t*>(udp_handle_.get()),
               nullptr);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                          // libuv API requirement
    }

    // イベントループをクローズ
    if (loop_) {
      uv_loop_close(loop_.get());
    }
  }
}

auto UDPSocket::IsValid() const -> bool {
  const std::lock_guard<std::mutex> kLock(mutex_);
  return is_valid_;
}

auto UDPSocket::GetLocalPort() const -> int {
  const std::lock_guard<std::mutex> kLock(mutex_);
  if (!is_valid_ || !udp_handle_) {
    return 0;
  }

  struct sockaddr_storage addr {};  // ゼロ初期化
  int addr_len = sizeof(addr);

  if (uv_udp_getsockname(
          udp_handle_.get(),
          reinterpret_cast<struct sockaddr*>(
              &addr),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                       // libuv API requirement
          &addr_len) != 0) {
    return 0;
  }

  if (addr.ss_family == AF_INET) {
    auto* addr_in = reinterpret_cast<struct sockaddr_in*>(
        &addr);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                 // libuv API requirement
    return ntohs(addr_in->sin_port);
  }

  return 0;
}

auto UDPSocket::SendTo(const std::vector<uint8_t>& data, const std::string& ip,
                       int port) -> bool {
  const std::lock_guard<std::mutex> kLock(mutex_);
  if (!is_valid_ || !udp_handle_ || data.empty()) {
    return false;
  }

  // 送信先アドレスを設定
  struct sockaddr_in dest_addr {};  // ゼロ初期化
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip.c_str(), &dest_addr.sin_addr) != 1) {
    return false;
  }

  // 送信リクエストを作成
  auto send_req = std::make_unique<uv_udp_send_t>();
  SendContext context;
  send_req->data = &context;

  // 送信バッファを設定
  // 注意: uv_buf_initはchar*を要求するが、送信時はデータを読み取るだけ
  // const_castは安全（libuvの古いAPI設計による制約）
  const uv_buf_t kBuf = uv_buf_init(  // NOLINT(misc-include-cleaner)
                                      // libuv公式API
      reinterpret_cast<char*>(const_cast<uint8_t*>(
          data.data())),  // NOLINT(cppcoreguidelines-pro-type-const-cast,cppcoreguidelines-pro-type-reinterpret-cast)
                          // libuv API requirement
      data.size());

  // 送信実行
  const int kResult = uv_udp_send(
      send_req.get(), udp_handle_.get(), &kBuf, 1,
      reinterpret_cast<const struct sockaddr*>(
          &dest_addr),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                        // libuv API requirement
      OnSendComplete);

  if (kResult != 0) {
    return false;
  }

  // イベントループを実行して送信完了を待つ
  while (!context.completed_ && (uv_run(loop_.get(), UV_RUN_ONCE) != 0)) {
    // 継続実行
  }

  // send_reqの所有権をコールバックに移管（OnSendCompleteで削除）
  send_req
      .release();  // NOLINT(bugprone-unused-return-value) コールバックで削除

  return context.success_;
}

auto UDPSocket::ReceiveFrom(std::vector<uint8_t>& data, std::string& sender_ip,
                            int& sender_port, int timeout_ms) -> bool {
  const std::lock_guard<std::mutex> kLock(mutex_);
  if (!is_valid_ || !udp_handle_) {
    return false;
  }

  ReceiveContext context;
  udp_handle_->data = &context;

  // 受信開始
  if (uv_udp_recv_start(udp_handle_.get(), AllocBuffer, OnReceive) != 0) {
    return false;
  }

  // タイムアウト設定
  auto start_time = std::chrono::steady_clock::now();

  // イベントループを実行して受信を待つ
  while (!context.completed_) {
    uv_run(loop_.get(), UV_RUN_NOWAIT);

    // タイムアウトチェック
    auto now = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    if (elapsed.count() >= timeout_ms) {
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // 受信停止
  uv_udp_recv_stop(udp_handle_.get());

  if (context.success_) {
    data = std::move(context.data_);
    sender_ip = std::move(context.sender_ip_);
    sender_port = context.sender_port_;
    return true;
  }

  return false;
}

// 静的コールバック関数
void UDPSocket::OnSendComplete(uv_udp_send_t* req, int status) {
  auto* context = static_cast<SendContext*>(req->data);
  context->completed_ = true;
  context->success_ = (status == 0);
  delete req;  // NOLINT(cppcoreguidelines-owning-memory) libuv callback
               // standard pattern
}

void UDPSocket::OnReceive(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf,
                          const struct sockaddr* addr, unsigned flags) {
  (void)flags;  // 未使用パラメータ
  auto* context = static_cast<ReceiveContext*>(handle->data);

  if (nread > 0 && addr != nullptr) {
    // データを保存（ポインタ演算の安全化）
    context->data_.assign(buf->base, std::next(buf->base, nread));

    // 送信者の情報を取得
    if (addr->sa_family == AF_INET) {
      const auto* addr_in = reinterpret_cast<const struct sockaddr_in*>(
          addr);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) libuv
                  // API requirement
      std::array<char, INET_ADDRSTRLEN> ip_str{};
      inet_ntop(AF_INET, &addr_in->sin_addr, ip_str.data(), INET_ADDRSTRLEN);
      context->sender_ip_ = ip_str.data();
      context->sender_port_ = ntohs(addr_in->sin_port);
    }

    context->success_ = true;
    context->completed_ = true;
  } else if (nread == 0) {
    // データなし、継続
  } else {
    // エラー
    context->success_ = false;
    context->completed_ = true;
  }

  if (buf->base != nullptr) {
    free(
        buf->base);  // NOLINT(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,hicpp-no-malloc)
                     // libuv buffer management
  }
}

void UDPSocket::AllocBuffer(uv_handle_t* handle, size_t suggested_size,
                            uv_buf_t* buf) {
  (void)handle;  // 未使用パラメータ
  buf->base = static_cast<char*>(malloc(
      suggested_size));  // NOLINT(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,hicpp-no-malloc)
                         // libuv buffer allocation
  buf->len = suggested_size;
}
