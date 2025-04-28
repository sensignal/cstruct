#!/bin/bash
# CStruct ライブラリの Arduino ビルドテストスクリプト
# Copyright (c) 2025 Sensignal Co.,Ltd.
# SPDX-License-Identifier: Apache-2.0

set -e  # エラーが発生したら即座に終了

# スクリプトのディレクトリを取得
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# プロジェクトのルートディレクトリを設定
PROJECT_ROOT="$SCRIPT_DIR/.."
# Arduino ライブラリディレクトリを設定
ARDUINO_LIB_DIR="$PROJECT_ROOT/arduino"
# Arduino サンプルディレクトリを設定
ARDUINO_EXAMPLES_DIR="$ARDUINO_LIB_DIR/examples"

# 色付きの出力用の変数
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# バージョン番号はテストビルドは0.0.0
VERSION="0.0.0"
# ボードを取得（引数から、デフォルトはArduino UNO）
BOARD=${1:-"arduino:avr:uno"}

echo -e "${YELLOW}CStruct ライブラリ バージョン ${VERSION} のテストを開始します${NC}"
echo -e "${YELLOW}ボード: ${BOARD}${NC}"

# Arduino-CLI が存在するか確認
if ! command -v arduino-cli &> /dev/null; then
    echo -e "${RED}エラー: arduino-cli が見つかりません。インストールしてください。${NC}"
    exit 1
fi

echo -e "${YELLOW}Arduino-CLI のバージョン:${NC}"
arduino-cli version

# Arduino-CLI 用の独立した環境を設定
export ARDUINO_DATA_DIR="$PROJECT_ROOT/_arduino-data"
export ARDUINO_DOWNLOAD_DIR="$PROJECT_ROOT/_arduino-downloads"
export ARDUINO_USER_DIR="$PROJECT_ROOT/_arduino-user"

# ディレクトリが存在しない場合は作成
mkdir -p "$ARDUINO_DATA_DIR" "$ARDUINO_DOWNLOAD_DIR" "$ARDUINO_USER_DIR"

echo -e "${YELLOW}Arduino-CLI 用の独立環境を作成しました:${NC}"
echo -e " - ARDUINO_DATA_DIR: $ARDUINO_DATA_DIR"
echo -e " - ARDUINO_DOWNLOAD_DIR: $ARDUINO_DOWNLOAD_DIR"
echo -e " - ARDUINO_USER_DIR: $ARDUINO_USER_DIR"

# Arduino-CLI の設定を初期化
echo -e "${YELLOW}Arduino-CLI の設定を初期化中...${NC}"
arduino-cli config init --overwrite

# Arduino-CLI の設定でallow_unsafe_installを許可
echo -e "${YELLOW}Arduino-CLI allow_unsafe_install...${NC}"
arduino-cli config set library.enable_unsafe_install true

# Arduino AVR コアをインストール
echo -e "${YELLOW}Arduino AVR コアをインストール中...${NC}"
arduino-cli core update-index
arduino-cli core install arduino:avr

# ボード指定からコア名を抽出
CORE_NAME=$(echo "$BOARD" | cut -d: -f1,2)

# コアが存在するか確認して、なければインストール
if ! arduino-cli core list | grep -q "^$CORE_NAME"; then
    echo -e "${YELLOW}コア ${CORE_NAME} が見つかりません。インストールします...${NC}"
    arduino-cli core update-index
    arduino-cli core install "$CORE_NAME"
else
    echo -e "${GREEN}コア ${CORE_NAME} はすでにインストールされています。${NC}"
fi

# パッケージを作成
echo -e "${YELLOW}CStruct パッケージを作成中...${NC}"
"$SCRIPT_DIR/create_package.sh" "$VERSION"

# ZIPパッケージのパスを設定
PACKAGE_ZIP="$PROJECT_ROOT/dist/cstruct-arduino-$VERSION.zip"

# ZIPファイルをインストール
echo -e "${YELLOW}パッケージをArduino-CLIでインストール中...${NC}"
arduino-cli lib install --zip-path "$PACKAGE_ZIP"

# サンプルスケッチをすべてコンパイル
EXAMPLES=$(find "$ARDUINO_EXAMPLES_DIR" -name "*.ino")
TOTAL_EXAMPLES=$(echo "$EXAMPLES" | wc -l)
TOTAL_EXAMPLES=$(echo "$TOTAL_EXAMPLES" | tr -d ' ')

echo -e "${YELLOW}合計 $TOTAL_EXAMPLES 個のサンプルスケッチをテストします...${NC}"

SUCCESS_COUNT=0
FAILED_EXAMPLES=""

for EXAMPLE in $EXAMPLES; do
    EXAMPLE_NAME=$(basename "$(dirname "$EXAMPLE")")
    echo -e "${YELLOW}サンプル「$EXAMPLE_NAME」をコンパイル中...${NC}"
    
    # スケッチをコンパイル（アップロードはしない）
    if arduino-cli compile --libraries="$ARDUINO_USER_DIR/libraries" --fqbn "$BOARD" "$EXAMPLE"; then
        echo -e "${GREEN}サンプル「$EXAMPLE_NAME」のコンパイルに成功しました。${NC}"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo -e "${RED}サンプル「$EXAMPLE_NAME」のコンパイルに失敗しました。${NC}"
        FAILED_EXAMPLES="$FAILED_EXAMPLES\n - $EXAMPLE_NAME"
    fi
done

# 一時ファイルを削除
echo -e "${YELLOW}一時ファイルを削除中...${NC}"
rm -f "$PACKAGE_ZIP"

# 結果を表示
echo -e "${YELLOW}ビルドテスト結果:${NC}"
echo -e "${GREEN}成功: $SUCCESS_COUNT / $TOTAL_EXAMPLES${NC}"

if [ "$SUCCESS_COUNT" -ne "$TOTAL_EXAMPLES" ]; then
    echo -e "${RED}失敗したサンプル:${FAILED_EXAMPLES}${NC}"
    exit 1
else
    echo -e "${GREEN}すべてのサンプルのコンパイルに成功しました！${NC}"
    exit 0
fi
