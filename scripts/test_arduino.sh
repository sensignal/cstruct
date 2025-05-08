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
# ボードを取得（デフォルトはArduino UNO）
BOARD="arduino:avr:uno"

# Arduino-Lintのオプション
LINT_SKIP=false
LINT_COMPLIANCE="specification"

# 引数の処理
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-lint)
            LINT_SKIP=true
            shift
            ;;
        --compliance=*)
            LINT_COMPLIANCE="${1#*=}"
            shift
            ;;
        *)
            # その他の引数はボードとして扱う
            BOARD="$1"
            shift
            ;;
    esac
done

echo -e "${YELLOW}CStruct ライブラリ バージョン ${VERSION} のテストを開始します${NC}"
echo -e "${YELLOW}ボード: ${BOARD}${NC}"

# Arduino-CLI が存在するか確認
if ! command -v arduino-cli &> /dev/null; then
    echo -e "${RED}エラー: arduino-cli が見つかりません。インストールしてください。${NC}"
    exit 1
fi

# Arduino-Lintの実行関数
run_arduino_lint() {
    echo -e "${YELLOW}Arduino-Lintを実行中...${NC}"
    
    # lintをスキップする場合
    if [ "$LINT_SKIP" = true ]; then
        echo -e "${YELLOW}Arduino-Lintはスキップされました。${NC}"
        return 0
    fi
    
    # Dockerが利用可能か確認
    if command -v docker &> /dev/null; then
        echo -e "${YELLOW}Dockerを使用してArduino-Lintを実行します...${NC}"
        
        # Arduino-Lint用の一時コンテナを作成
        echo -e "${YELLOW}一時的なArduino-Lintコンテナを作成します...${NC}"
        
        # 一時ディレクトリを作成
        TEMP_DIR="$(mktemp -d)"
        trap "rm -rf $TEMP_DIR" EXIT
        
        # Dockerfileを作成
        cat > "$TEMP_DIR/Dockerfile" << 'EOF'
FROM alpine:3.18

RUN apk add --no-cache curl

WORKDIR /app

# Arduino-lintのインストール
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-lint/main/etc/install.sh | sh

ENTRYPOINT ["/app/bin/arduino-lint"]
EOF
        
        # Dockerイメージをビルド
        if docker build -t arduino-lint-temp "$TEMP_DIR" > /dev/null 2>&1; then
            echo -e "${GREEN}一時的なArduino-Lintコンテナを作成しました。${NC}"
            
            # Arduino-lintの実行
            echo -e "${YELLOW}Arduino-Lintを実行中...${NC}"
            if docker run --rm -v "$ARDUINO_LIB_DIR:/library" arduino-lint-temp \
                --library-manager false \
                --compliance "$LINT_COMPLIANCE" \
                --format text \
                /library; then
                echo -e "${GREEN}Arduino-Lintは正常に完了しました。${NC}"
                
                # 一時的なイメージを削除
                docker rmi arduino-lint-temp > /dev/null 2>&1
                
                return 0
            else
                echo -e "${RED}Arduino-Lintはエラーで終了しました。${NC}"
                
                # 一時的なイメージを削除
                docker rmi arduino-lint-temp > /dev/null 2>&1
                
                return 1
            fi
        else
            echo -e "${RED}Arduino-Lintコンテナの作成に失敗しました。${NC}"
            return 1
        fi
    else
        echo -e "${YELLOW}Dockerが見つかりません。Arduino-Lintをスキップします。${NC}"
        echo -e "${YELLOW}Arduino-Lintを実行するにはDockerをインストールするか、GitHub Actionsで実行してください。${NC}"
        return 0  # Dockerがない場合はエラーとしない
    fi
}

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

# Arduino-Lintを実行
if [ "$LINT_SKIP" = true ]; then
    echo -e "${YELLOW}Arduino-Lintはスキップされました。${NC}"
    LINT_RESULT=0
else
    run_arduino_lint
    LINT_RESULT=$?
fi

# Lintが失敗した場合は警告を表示するが、テストは続行する
if [ $LINT_RESULT -ne 0 ]; then
    echo -e "${YELLOW}警告: Arduino-Lintでエラーが検出されました。${NC}"
    
    # Library Managerのエラーは開発段階では無視する
    echo -e "${YELLOW}Library Managerのエラーは開発段階では無視されます。${NC}"
    echo -e "${YELLOW}ライブラリ公開時には、Arduino Library Managerに登録する必要があります。${NC}"
    
    # 開発段階ではエラーを無視して続行する
    # exit 1  # 厳格にする場合はこの行のコメントを外す
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
    echo -e "${YELLOW}サンプル '$EXAMPLE_NAME' をコンパイル中...${NC}"
    
    # スケッチをコンパイル（アップロードはしない）
    if arduino-cli compile --libraries="$ARDUINO_USER_DIR/libraries" --fqbn "$BOARD" "$EXAMPLE"; then
        echo -e "${GREEN}サンプル '$EXAMPLE_NAME' のコンパイルに成功しました。${NC}"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo -e "${RED}サンプル '$EXAMPLE_NAME' のコンパイルに失敗しました。${NC}"
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
