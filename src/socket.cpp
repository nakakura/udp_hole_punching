#include "socket.h"

UDPSocket::UDPSocket() : socket_fd_(-1)
{
    // TODO: UDPソケット作成を実装
    // まずはRed状態にするため、実装しない
}

UDPSocket::~UDPSocket()
{
    // TODO: ソケットクローズを実装
}

bool UDPSocket::isValid() const
{
    // TODO: ソケット有効性チェックを実装
    // まずはRed状態にするため、常にfalseを返す
    return false;
}

int UDPSocket::getFd() const
{
    return socket_fd_;
}
