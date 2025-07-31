# UDP Hole Punching Implementation (C++)

UDP Hole Punching技術の学習を目的としたC++実装プロジェクトです。

## 概要

UDP Hole Punchingは、NATやファイアウォールの背後にある2つのクライアント間で直接的なUDP通信を確立するための技術です。
これは自身の学習用の実装です。

## 開発方針

### Step 1: STUNクライアント実装
- RFC 5389に準拠したSTUNクライアントを実装
- Google Public STUN Server (stun.l.google.com:19302) との通信
- 自分のパブリックIPアドレス・ポート番号の取得

### Step 2: Rendezvous Server実装
- クライアント間の情報仲介を行うカスタムサーバー
- ピア情報の管理・交換
- Hole Punchingのタイミング調整

### Step 3: UDP Hole Punching完成
- STUNで取得した情報をRendezvous Serverで交換
- 実際のHole Punching実行
- P2P直接通信の確立

## 技術スタック

- **言語**: C++17
- **ビルドシステム**: CMake
- **ネットワーク**: Berkeley Sockets (POSIX)
- **プロトコル**: STUN ([RFC 5389](https://datatracker.ietf.org/doc/html/rfc5389)), 独自Rendezvousプロトコル
- **ライブラリ方針**: 学習目的のため、libwebrtc等の大規模ライブラリは使用せず、Boost等の標準的なライブラリのみを使用

## プロジェクト構成

```
udp_hole_punching/
├── include/          # ヘッダーファイル
├── src/              # ソースファイル  
├── examples/         # 使用例・テストプログラム
├── docs/             # ドキュメント
└── CMakeLists.txt    # ビルド設定
```

## 学習ポイント

1. **STUNプロトコル**: NAT環境での自己アドレス発見
2. **ソケットプログラミング**: UDPソケットの非同期処理
3. **NAT Traversal**: ファイアウォール・NAT越え技術
4. **P2P通信**: サーバーレス直接通信の実現
5. **プロトコル実装**: 既存ライブラリに依存しない低レベル実装の理解

## 参考文献

### RFC・標準仕様
- [RFC 5389: Session Traversal Utilities for NAT (STUN)](https://datatracker.ietf.org/doc/html/rfc5389)
- [RFC 5245: Interactive Connectivity Establishment (ICE)](https://datatracker.ietf.org/doc/html/rfc5245)
- [RFC 3489: STUN - Simple Traversal of UDP through NATs (旧版)](https://datatracker.ietf.org/doc/html/rfc3489)

### 技術解説
- [NAT Traversal Techniques](https://tools.ietf.org/html/draft-ford-natp2p-01)
- [P2P communication across middleboxes](https://bford.info/pub/net/p2pnat/)

---

詳細な進捗管理は [GitHub Issues](../../issues) で行います。
