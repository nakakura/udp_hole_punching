#!/bin/bash

# 開発環境自動セットアップスクリプト
# 新メンバーやドキュメントを読まないメンバー用

set -e

echo "🚀 UDP Hole Punching プロジェクト 開発環境セットアップ"
echo "=================================================="

# 0. 開発ツールのインストール
echo "📦 開発ツールのインストール中..."

# clang-format と clang-tidy のインストール
if ! command -v clang-format &> /dev/null; then
    echo "📥 clang-formatをインストール中..."
    sudo apt update && sudo apt install -y clang-format
    echo "✅ clang-formatをインストールしました"
else
    echo "✅ clang-formatは既にインストール済みです"
fi

if ! command -v clang-tidy &> /dev/null; then
    echo "📥 clang-tidyをインストール中..."
    sudo apt install -y clang-tidy
    echo "✅ clang-tidyをインストールしました"
else
    echo "✅ clang-tidyは既にインストール済みです"
fi

# pre-commit のインストール
if ! command -v pre-commit &> /dev/null; then
    echo "📥 pre-commitをインストール中..."
    # Ubuntu 24.04では外部管理環境のためaptでインストール
    sudo apt install -y pre-commit
    echo "✅ pre-commitをインストールしました"
else
    echo "✅ pre-commitは既にインストール済みです"
fi

echo ""

# 1. commit-msgフックのセットアップ
COMMIT_MSG_SOURCE="scripts/hooks/commit-msg"
COMMIT_MSG_TARGET=".git/hooks/commit-msg"

if [ -f "$COMMIT_MSG_TARGET" ]; then
    echo "📋 commit-msgフックは既に存在します"
    if [ "$COMMIT_MSG_SOURCE" -nt "$COMMIT_MSG_TARGET" ]; then
        echo "🔄 新しいバージョンが利用可能です。更新中..."
        cp "$COMMIT_MSG_SOURCE" "$COMMIT_MSG_TARGET"
        chmod +x "$COMMIT_MSG_TARGET"
        echo "✅ commit-msgフックを更新しました"
    else
        echo "✅ commit-msgフックは最新です"
    fi
else
    echo "📥 commit-msgフックをセットアップ中..."
    if [ -f "$COMMIT_MSG_SOURCE" ]; then
        cp "$COMMIT_MSG_SOURCE" "$COMMIT_MSG_TARGET"
        chmod +x "$COMMIT_MSG_TARGET"
        echo "✅ commit-msgフックをセットアップしました"
    else
        echo "❌ エラー: $COMMIT_MSG_SOURCE が見つかりません"
        echo "   プロジェクトが正しくセットアップされていない可能性があります"
        exit 1
    fi
fi

# 2. pre-commitフックのセットアップ（オプション）
PRE_COMMIT_SOURCE="scripts/hooks/pre-commit"
PRE_COMMIT_TARGET=".git/hooks/pre-commit"

if [ -f "$PRE_COMMIT_SOURCE" ]; then
    echo ""
    echo "🔧 pre-commitフック（開発環境チェック）もセットアップしますか? [y/N]"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        cp "$PRE_COMMIT_SOURCE" "$PRE_COMMIT_TARGET"
        chmod +x "$PRE_COMMIT_TARGET"
        echo "✅ pre-commitフックもセットアップしました"
        echo "   今後コミット時に開発環境設定を自動チェックします"
    fi
fi

# 3. pre-commit フレームワークのセットアップ
echo ""
echo "🔧 pre-commitフレームワークをセットアップ中..."
if [ -f ".pre-commit-config.yaml" ]; then
    pre-commit install
    echo "✅ pre-commitフックがセットアップされました"

    echo "🧪 pre-commitの動作確認中..."
    if pre-commit run --all-files; then
        echo "✅ pre-commitが正常に動作しています"
    else
        echo "⚠️  一部のファイルがフォーマットされました（正常な動作です）"
    fi
else
    echo "❌ .pre-commit-config.yamlが見つかりません"
fi

# 4. 動作確認
echo ""
echo "🧪 フックの動作確認中..."
if [ -f "$COMMIT_MSG_TARGET" ]; then
    echo "feat(test): セットアップテスト" | "$COMMIT_MSG_TARGET" /dev/stdin
    if [ $? -eq 0 ]; then
        echo "✅ commit-msgフックが正常に動作しています"
    else
        echo "❌ commit-msgフックに問題があります"
        exit 1
    fi
else
    echo "❌ commit-msgフックがセットアップされていません"
    exit 1
fi

# 5. 完了メッセージ
echo ""
echo "🎉 セットアップ完了！"
echo ""
echo "📖 重要な情報:"
echo "  • Conventional Commitsフォーマットでコミットしてください"
echo "  • 詳細な開発手順はDEVELOPMENT.mdを確認してください"
echo "  • テスト実行: make test または適切なビルドコマンド"
echo "  • コードフォーマット: clang-format --style=file -i <ファイル名>"
echo "  • 静的解析: clang-tidy <ファイル名>"
echo "  • pre-commit手動実行: pre-commit run --all-files"
echo ""
echo "🔍 コミットメッセージ形式例:"
echo "  feat(udp): 新しいUDP機能を追加"
echo "  fix(test): テストのバグを修正"
echo "  docs: READMEを更新"
echo ""
echo "Happy coding! 🚀"
