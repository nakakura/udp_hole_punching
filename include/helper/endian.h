#ifndef HELPER_ENDIAN_H
#define HELPER_ENDIAN_H

#include <cstdint>

/**
 * エンディアン変換関数
 *
 * ネットワークプロトコル実装で使用するバイトオーダー変換機能を提供
 * RFC準拠のSTUN/TURN/ICE実装に必要なエンディアン変換を行う
 */
namespace endian {
/**
 * 16bit値をホストバイトオーダーからネットワークバイトオーダーに変換
 *
 * htons()相当の機能を勉強用に独自実装する
 *
 * @param bytes ホストバイトオーダーのバイト配列（最低2バイト必要）
 * @return ネットワークバイトオーダー（ビッグエンディアン）の16bit値
 */
auto HostToNetwork16(const uint8_t* bytes) -> uint16_t;

/**
 * 32bit値をホストバイトオーダーからネットワークバイトオーダーに変換
 *
 * htonl()相当の機能を勉強用に独自実装する
 *
 * @param bytes ホストバイトオーダーのバイト配列（最低4バイト必要）
 * @return ネットワークバイトオーダー（ビッグエンディアン）の32bit値
 */
auto HostToNetwork32(const uint8_t* bytes) -> uint32_t;

/**
 * バイト配列から16bit値をネットワークバイトオーダーで読み取り
 *
 * ntohs()相当の機能を勉強用に独自実装する
 *
 * @param bytes ネットワークバイトオーダーのバイト配列（最低2バイト必要）
 * @return ホストバイトオーダーの16bit値
 */
auto NetworkToHost16(const uint8_t* bytes) -> uint16_t;

/**
 * バイト配列から32bit値をネットワークバイトオーダーで読み取り
 *
 * ntohl()相当の機能を勉強用に独自実装する
 *
 * @param bytes ネットワークバイトオーダーのバイト配列（最低4バイト必要）
 * @return ホストバイトオーダーの32bit値
 */
auto NetworkToHost32(const uint8_t* bytes) -> uint32_t;
}  // namespace endian

#endif  // HELPER_ENDIAN_H
