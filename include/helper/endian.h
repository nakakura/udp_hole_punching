#pragma once

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
uint16_t host_to_network_16(const uint8_t* bytes);

/**
 * 32bit値をホストバイトオーダーからネットワークバイトオーダーに変換
 *
 * htonl()相当の機能を勉強用に独自実装する
 *
 * @param bytes ホストバイトオーダーのバイト配列（最低4バイト必要）
 * @return ネットワークバイトオーダー（ビッグエンディアン）の32bit値
 */
uint32_t host_to_network_32(const uint8_t* bytes);

/**
 * バイト配列から16bit値をネットワークバイトオーダーで読み取り
 *
 * ntohs()相当の機能を勉強用に独自実装する
 *
 * @param bytes ネットワークバイトオーダーのバイト配列（最低2バイト必要）
 * @return ホストバイトオーダーの16bit値
 */
uint16_t network_to_host_16(const uint8_t* bytes);

/**
 * バイト配列から32bit値をネットワークバイトオーダーで読み取り
 *
 * ntohl()相当の機能を勉強用に独自実装する
 *
 * @param bytes ネットワークバイトオーダーのバイト配列（最低4バイト必要）
 * @return ホストバイトオーダーの32bit値
 */
uint32_t network_to_host_32(const uint8_t* bytes);
}  // namespace endian
