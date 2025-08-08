# 開発環境セットアップガイド

## 🚀 クイックスタート（推奨）

このプロジェクトでは、開発環境の自動セットアップスクリプトを提供しています。

### 自動セットアップ

```bash
# プロジェクトルートで実行
./setup-dev.sh
```

このスクリプトが以下を自動で行います：
- Conventional Commitsフックのセットアップ
- pre-commitフック（開発環境チェック）のセットアップ（オプション）
- フックの動作確認
- セットアップ完了の確認

### 手動セットアップ（上級者向け）

自動セットアップが何らかの理由で使用できない場合：

1. **commit-msgフックをローカルにコピー:**
   ```bash
   cp scripts/hooks/commit-msg .git/hooks/commit-msg
   chmod +x .git/hooks/commit-msg
   ```

2. **pre-commitフック（オプション）:**
   ```bash
   cp scripts/hooks/pre-commit .git/hooks/pre-commit
   chmod +x .git/hooks/pre-commit
   ```

3. **動作確認:**
   ```bash
   # 正しい形式でテスト
   echo "feat(test): 新しいテスト機能を追加" | .git/hooks/commit-msg /dev/stdin

   # 間違った形式でテスト
   echo "間違ったコミットメッセージ" | .git/hooks/commit-msg /dev/stdin
   ```

## 🛡️ 自動保護機能

### pre-commitフック

pre-commitフックをセットアップした場合、以下の保護機能が有効になります：

- **開発環境チェック**: コミット時にcommit-msgフックの設定状況を自動確認
- **自動セットアップ提案**: 未設定の場合、自動セットアップを提案
- **フック更新通知**: 古いバージョンのフックを検出して更新を提案

### フックの更新

プロジェクトのフックが更新された場合：

```bash
# 自動検出・更新（pre-commitフック有効時）
git commit -m "feat: 何らかの変更"
# ↓ 古いフックを検出して更新を提案

# 手動更新
./setup-dev.sh
```

## 📋 Conventional Commits形式

- **形式:** `<type>[optional scope]: <description>`
- **例:**
  - `feat(udp): UDPソケットの送信機能を追加`
  - `fix(test): テストのタイムアウト問題を修正`
  - `docs: READMEを更新`

### 使用可能なtype一覧

| type     | 説明                           |
|----------|--------------------------------|
| feat     | 新機能の追加                   |
| fix      | バグ修正                       |
| docs     | ドキュメントの変更             |
| style    | コードフォーマット             |
| refactor | リファクタリング               |
| test     | テストの追加・修正             |
| chore    | ビルドプロセスやツールの変更   |
| perf     | パフォーマンス改善             |
| ci       | CI設定の変更                   |
| build    | ビルドシステムの変更           |
| revert   | コミットの取り消し             |

## ⚠️ トラブルシューティング

### コミットが失敗する場合

1. **commit-msgフック未設定**
   ```
   ❌ 開発環境が正しく設定されていません！
   ```
   → `./setup-dev.sh` を実行してください

2. **コミットメッセージ形式エラー**
   ```
   ❌ コミットメッセージがConventional Commits形式に従っていません
   ```
   → 下記の形式に従ってコミットメッセージを修正してください

3. **フックファイルが見つからない**
   ```
   ❌ エラー: scripts/hooks/commit-msg が見つかりません
   ```
   → プロジェクトが正しくcloneされているか確認してください

### セットアップスクリプトが動作しない場合

```bash
# 実行権限の確認・付与
chmod +x setup-dev.sh

# スクリプトの存在確認
ls -la setup-dev.sh

# 手動セットアップに切り替え
cp scripts/hooks/commit-msg .git/hooks/commit-msg
chmod +x .git/hooks/commit-msg
```

## 🎯 新メンバー向けチェックリスト

- [ ] リポジトリをclone
- [ ] `./setup-dev.sh` を実行
- [ ] テストコミットで動作確認
- [ ] DEVELOPMENT.mdを確認
- [ ] プロジェクトの技術スタックを理解

### 重要事項

- コミットメッセージの1行目は72文字以内
- マージコミットは自動的に除外されます
- フックが失敗した場合、コミットは中止されます
- **setup-dev.shを実行しないとコミットできません**

## 📚 関連ドキュメント

- [README.md](README.md) - プロジェクト概要
- [scripts/hooks/](scripts/hooks/) - フックファイル
- [Conventional Commits仕様](https://www.conventionalcommits.org/ja/)

---

**重要**: 開発を始める前に必ず `./setup-dev.sh` を実行してください！
