#include "udp_socket.h"
#include <cstring>
#include <chrono>
#include <thread>
#include <arpa/inet.h>

UDPSocket::UDPSocket() : loop_(std::make_unique<uv_loop_t>()),
                         udp_handle_(std::make_unique<uv_udp_t>()),
                         is_valid_(false)
{
    // libuvイベントループを初期化
    if (uv_loop_init(loop_.get()) != 0)
    {
        return;
    }

    // UDPハンドルを初期化
    if (uv_udp_init(loop_.get(), udp_handle_.get()) != 0)
    {
        uv_loop_close(loop_.get());
        return;
    }

    // UDPソケットを任意のポートにバインド（ファイルディスクリプタ取得のため）
    struct sockaddr_in bind_addr;
    std::memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = 0; // OSが自動でポート選択

    if (uv_udp_bind(udp_handle_.get(),
                    reinterpret_cast<const struct sockaddr *>(&bind_addr),
                    0) != 0)
    {
        uv_close(reinterpret_cast<uv_handle_t *>(udp_handle_.get()), nullptr);
        uv_loop_close(loop_.get());
        return;
    }

    is_valid_ = true;
}

UDPSocket::~UDPSocket()
{
    if (is_valid_)
    {
        // UDPハンドルをクローズ
        if (udp_handle_)
        {
            uv_close(reinterpret_cast<uv_handle_t *>(udp_handle_.get()), nullptr);
        }

        // イベントループをクローズ
        if (loop_)
        {
            uv_loop_close(loop_.get());
        }
    }
}

bool UDPSocket::isValid() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return is_valid_;
}

int UDPSocket::getLocalPort() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_valid_ || !udp_handle_)
    {
        return 0;
    }

    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);

    if (uv_udp_getsockname(udp_handle_.get(),
                           reinterpret_cast<struct sockaddr *>(&addr),
                           &addr_len) != 0)
    {
        return 0;
    }

    if (addr.ss_family == AF_INET)
    {
        struct sockaddr_in *addr_in = reinterpret_cast<struct sockaddr_in *>(&addr);
        return ntohs(addr_in->sin_port);
    }

    return 0;
}

bool UDPSocket::sendTo(const std::vector<uint8_t> &data, const std::string &ip, int port)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_valid_ || !udp_handle_ || data.empty())
    {
        return false;
    }

    // 送信先アドレスを設定
    struct sockaddr_in dest_addr;
    std::memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &dest_addr.sin_addr) != 1)
    {
        return false;
    }

    // 送信リクエストを作成
    auto send_req = std::make_unique<uv_udp_send_t>();
    SendContext context;
    send_req->data = &context;

    // 送信バッファを設定
    uv_buf_t buf = uv_buf_init(const_cast<char *>(reinterpret_cast<const char *>(data.data())), data.size());

    // 送信実行
    int result = uv_udp_send(send_req.get(), udp_handle_.get(), &buf, 1,
                             reinterpret_cast<const struct sockaddr *>(&dest_addr),
                             onSendComplete);

    if (result != 0)
    {
        return false;
    }

    // イベントループを実行して送信完了を待つ
    while (!context.completed && uv_run(loop_.get(), UV_RUN_ONCE))
        ;

    // send_reqの所有権を維持するため、完了まで保持
    send_req.release(); // メモリリークを防ぐため、コールバックで削除

    return context.success;
}

bool UDPSocket::receiveFrom(std::vector<uint8_t> &data, std::string &sender_ip, int &sender_port, int timeout_ms)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_valid_ || !udp_handle_)
    {
        return false;
    }

    ReceiveContext context;
    udp_handle_->data = &context;

    // 受信開始
    if (uv_udp_recv_start(udp_handle_.get(), allocBuffer, onReceive) != 0)
    {
        return false;
    }

    // タイムアウト設定
    auto start_time = std::chrono::steady_clock::now();

    // イベントループを実行して受信を待つ
    while (!context.completed)
    {
        uv_run(loop_.get(), UV_RUN_NOWAIT);

        // タイムアウトチェック
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
        if (elapsed.count() >= timeout_ms)
        {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // 受信停止
    uv_udp_recv_stop(udp_handle_.get());

    if (context.success)
    {
        data = std::move(context.data);
        sender_ip = std::move(context.sender_ip);
        sender_port = context.sender_port;
        return true;
    }

    return false;
}

// 静的コールバック関数
void UDPSocket::onSendComplete(uv_udp_send_t *req, int status)
{
    SendContext *context = static_cast<SendContext *>(req->data);
    context->completed = true;
    context->success = (status == 0);
    delete req; // メモリ解放
}

void UDPSocket::onReceive(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
                          const struct sockaddr *addr, unsigned flags)
{
    (void)flags; // 未使用パラメータ
    ReceiveContext *context = static_cast<ReceiveContext *>(handle->data);

    if (nread > 0 && addr != nullptr)
    {
        // データを保存
        context->data.assign(buf->base, buf->base + nread);

        // 送信者の情報を取得
        if (addr->sa_family == AF_INET)
        {
            const struct sockaddr_in *addr_in = reinterpret_cast<const struct sockaddr_in *>(addr);
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, INET_ADDRSTRLEN);
            context->sender_ip = ip_str;
            context->sender_port = ntohs(addr_in->sin_port);
        }

        context->success = true;
        context->completed = true;
    }
    else if (nread == 0)
    {
        // データなし、継続
    }
    else
    {
        // エラー
        context->success = false;
        context->completed = true;
    }

    if (buf->base)
    {
        free(buf->base);
    }
}

void UDPSocket::allocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    (void)handle; // 未使用パラメータ
    buf->base = static_cast<char *>(malloc(suggested_size));
    buf->len = suggested_size;
}
