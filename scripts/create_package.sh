#!/bin/bash
# CStructパッケージ作成スクリプト
# このスクリプトはArduinoのパッケージを作成します

# エラーが発生したら終了
set -e

# バージョン番号の取得（引数から取得、なければ手動入力）
if [ -z "$1" ]; then
  echo "バージョン番号を入力してください（例: 1.0.0）:"
  read VERSION
else
  VERSION=$1
fi

echo "パッケージバージョン: $VERSION を作成します"

# 作業ディレクトリをスクリプトの場所に設定
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

# 出力ディレクトリの作成
OUTPUT_DIR="$PROJECT_ROOT/dist"
mkdir -p "$OUTPUT_DIR"

echo "===== Arduinoパッケージの作成 ====="
# Arduinoパッケージ用の一時ディレクトリ
ARDUINO_TMP_DIR="$PROJECT_ROOT/tmp_arduino_package"
mkdir -p "$ARDUINO_TMP_DIR/CStruct/src"

# Arduinoディレクトリからファイルをコピー
cp "$PROJECT_ROOT/arduino/src/CStruct.cpp" "$ARDUINO_TMP_DIR/CStruct/src/"
cp "$PROJECT_ROOT/arduino/src/CStruct.h" "$ARDUINO_TMP_DIR/CStruct/src/"
cp "$PROJECT_ROOT/arduino/keywords.txt" "$ARDUINO_TMP_DIR/CStruct/"
cp "$PROJECT_ROOT/arduino/README.md" "$ARDUINO_TMP_DIR/CStruct/"

# C言語版のファイルもコピー（Arduino版はC言語版に依存しているため）
mkdir -p "$ARDUINO_TMP_DIR/CStruct/src/cstruct"
cp "$PROJECT_ROOT/c/cstruct.c" "$ARDUINO_TMP_DIR/CStruct/src/cstruct/"
cp "$PROJECT_ROOT/c/cstruct.h" "$ARDUINO_TMP_DIR/CStruct/src/cstruct/"

# CStruct.cppのインクルードパスを修正
sed -i'.bak' 's|#include "../../c/cstruct.h"|#include "cstruct/cstruct.h"|g' "$ARDUINO_TMP_DIR/CStruct/src/CStruct.cpp"
rm -f "$ARDUINO_TMP_DIR/CStruct/src/CStruct.cpp.bak"

# library.propertiesをコピーしてバージョン番号を更新
cat "$PROJECT_ROOT/arduino/library.properties" | sed "s/version=1.0.0/version=$VERSION/g" > "$ARDUINO_TMP_DIR/CStruct/library.properties"

# Arduinoサンプルコードのコピー
if [ -d "$PROJECT_ROOT/arduino/examples" ]; then
  cp -r "$PROJECT_ROOT/arduino/examples" "$ARDUINO_TMP_DIR/CStruct/"
else
  echo "Arduinoサンプルが見つかりません。"
  mkdir -p "$ARDUINO_TMP_DIR/CStruct/examples/basic"
fi

# ZIP アーカイブの作成
cd "$ARDUINO_TMP_DIR"
zip -r "$OUTPUT_DIR/cstruct-arduino-$VERSION.zip" CStruct
cd "$PROJECT_ROOT"
rm -rf "$ARDUINO_TMP_DIR"

echo "===== パッケージ作成完了 ====="
echo "パッケージは以下の場所に作成されました:"
echo "$OUTPUT_DIR/cstruct-arduino-$VERSION.zip"
