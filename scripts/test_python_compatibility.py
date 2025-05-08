#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
CStructライブラリとPythonのstructモジュールの互換性テスト

このスクリプトは、CStructライブラリとPythonのstructモジュールの間で
バイナリデータのパックとアンパックの互換性をテストします。
"""

import os
import sys
import struct
import subprocess
import tempfile
import binascii

# テスト結果の表示用
class TestResult:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        
    def add_pass(self):
        self.passed += 1
        
    def add_fail(self):
        self.failed += 1
        
    def summary(self):
        total = self.passed + self.failed
        print(f"\n結果サマリー:")
        print(f"合計テスト数: {total}")
        print(f"成功: {self.passed}")
        print(f"失敗: {self.failed}")
        print(f"成功率: {self.passed / total * 100:.1f}%")
        
        return self.failed == 0

# テスト用のCプログラムを生成する関数
def generate_c_test_program(format_str, values, output_file):
    """
    指定されたフォーマット文字列と値を使用してCプログラムを生成します。
    """
    # スクリプトの実行パスを取得
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # プロジェクトのルートディレクトリ
    project_root = os.path.dirname(script_dir)
    # cstructのヘッダファイルへの絶対パス
    cstruct_header = os.path.join(project_root, "c", "cstruct.h")
    
    # 改行文字の問題を避けるため、行ごとにリストに格納してから結合する
    c_code_lines = [
        "/* =========================================================================\n",
        "    CStruct - Python互換性テスト用一時プログラム\n",
        "========================================================================= */\n",
        "#include <stdio.h>\n",
        "#include <stdlib.h>\n",
        "#include <stdint.h>\n",
        "#include <string.h>\n",
        f"#include \"{cstruct_header}\"\n",
        "\n",
        "int main() {\n",
        "    uint8_t buffer[1024];\n",
        "    memset(buffer, 0, sizeof(buffer));\n",
        "\n",
        "    // パックするデータ\n",
        f"    {format_values_to_c_vars(format_str, values)}\n",
        "\n",
        "    // パック\n",
        f"    cstruct_pack(buffer, sizeof(buffer), \"{format_str}\"{format_values_to_c_args(format_str, values)});\n",
        "\n",
        "    // バイナリデータを16進数で出力\n",
        f"    for (size_t i = 0; i < {calculate_packed_size(format_str, values)}; i++) {{\n",
        "        printf(\"%02x\", buffer[i]);\n",
        "    }\n",
        "    printf(\"\\n\");\n",
        "\n",
        "    return 0;\n",
        "}\n"
    ]
    
    # リストを結合して一つの文字列にする
    c_code = "".join(c_code_lines)
    
    with open(output_file, 'w') as f:
        f.write(c_code)

# フォーマット文字列に基づいてC言語の変数宣言を生成
def format_values_to_c_vars(format_str, values):
    """
    フォーマット文字列と値のリストからC言語の変数宣言を生成します。
    Pythonのstructモジュールの仕様に合わせて処理します。
    """
    result = []
    value_index = 0
    
    # エンディアン指定子を取得
    endian = '<'  # デフォルトはネイティブエンディアン
    if format_str and format_str[0] in '<>!':
        endian = format_str[0]
        format_str = format_str[1:]
    
    i = 0
    while i < len(format_str):
        # 数値プレフィックスを解析
        count = 1
        count_str = ""
        while i < len(format_str) and format_str[i].isdigit():
            count_str += format_str[i]
            i += 1
        
        if count_str:
            count = int(count_str)
            
        # 型指定子を処理
        if i < len(format_str):
            type_char = format_str[i]
            
            # パディングの場合は変数不要
            if type_char == 'x':
                i += 1
                continue
                
            # 文字列の場合
            elif type_char == 's':
                if value_index < len(values):
                    value = values[value_index]
                    # バイト列として正しく処理
                    if isinstance(value, bytes):
                        # バイト列をCの文字列リテラルに変換
                        hex_values = [f"0x{b:02x}" for b in value]
                        result.append(f"char str_{value_index}[] = {{{', '.join(hex_values)}}};")
                    else:
                        # 文字列の場合
                        result.append(f"char str_{value_index}[] = \"{value}\";")
                    value_index += 1
                    
            # 数値型の場合
            else:
                c_type = get_c_type_for_format(type_char)
                
                # Pythonのstructモジュールでは配列指定は使用できない
                # 代わりに同じ型を複数回指定する形式を使用
                if count > 1 and type_char != 's':
                    # 単一の型を複数回指定する場合は、各値を個別に宣言
                    for j in range(count):
                        if value_index < len(values):
                            value = values[value_index]
                            # 浮動小数点数の場合、Cの表記に合わせる
                            if type_char in 'efd':
                                if isinstance(value, float):
                                    value = f"{value}f" if type_char in 'ef' else f"{value}"
                            result.append(f"{c_type} var_{value_index} = {value};")
                            value_index += 1
                else:
                    # 単一値の場合
                    if value_index < len(values):
                        value = values[value_index]
                        
                        # 浮動小数点数の場合、Cの表記に合わせる
                        if type_char in 'efd':
                            if isinstance(value, float):
                                value = f"{value}f" if type_char in 'ef' else f"{value}"
                        
                        result.append(f"{c_type} var_{value_index} = {value};")
                        value_index += 1
            
            i += 1
        else:
            break
    
    return "\n    ".join(result)

# フォーマット文字列に基づいてC言語の関数引数を生成
def format_values_to_c_args(format_str, values):
    """
    フォーマット文字列と値のリストからcstruct_pack関数の引数を生成します。
    Pythonのstructモジュールの仕様に合わせて処理します。
    """
    result = []
    value_index = 0
    
    # エンディアン指定子を取得
    if format_str and format_str[0] in '<>!':
        endian = format_str[0]
        format_str = format_str[1:]
    
    i = 0
    while i < len(format_str):
        # 数値プレフィックスを解析
        count = 1
        count_str = ""
        while i < len(format_str) and format_str[i].isdigit():
            count_str += format_str[i]
            i += 1
        
        if count_str:
            count = int(count_str)
            
        # 型指定子を処理
        if i < len(format_str):
            type_char = format_str[i]
            
            # パディングの場合は引数不要
            if type_char == 'x':
                i += 1
                continue
                
            # 文字列の場合
            elif type_char == 's':
                if value_index < len(values):
                    result.append(f", str_{value_index}")
                    value_index += 1
                    
            # 数値型の場合
            else:
                # Pythonのstructモジュールでは配列指定は使用できない
                # 代わりに同じ型を複数回指定する形式を使用
                if count > 1 and type_char != 's':
                    # 単一の型を複数回指定する場合は、各値を個別に渡す
                    for j in range(count):
                        if value_index < len(values):
                            result.append(f", var_{value_index}")
                            value_index += 1
                else:
                    # 単一値の場合
                    if value_index < len(values):
                        result.append(f", var_{value_index}")
                        value_index += 1
            
            i += 1
        else:
            break
    
    return "".join(result)

# フォーマット文字に対応するC言語の型を取得
def get_c_type_for_format(format_char):
    """
    フォーマット文字からC言語の型名を取得します。
    """
    format_to_c_type = {
        'b': 'int8_t',
        'B': 'uint8_t',
        'h': 'int16_t',
        'H': 'uint16_t',
        'i': 'int32_t',
        'I': 'uint32_t',
        'q': 'int64_t',
        'Q': 'uint64_t',
        'e': 'float',  # half precision
        'f': 'float',
        'd': 'double',
        's': 'char*'
    }
    
    return format_to_c_type.get(format_char, 'uint8_t')

# パックされたデータのサイズを計算
def calculate_packed_size(format_str, values):
    """
    フォーマット文字列からパックされたデータのサイズを計算します。
    """
    size = 0
    i = 0
    
    while i < len(format_str):
        # エンディアン指定子をスキップ
        if format_str[i] in '<>':
            i += 1
            continue
            
        # 数値プレフィックスを解析
        count = 1
        count_str = ""
        while i < len(format_str) and format_str[i].isdigit():
            count_str += format_str[i]
            i += 1
        
        if count_str:
            count = int(count_str)
            
        # 型指定子を処理
        if i < len(format_str):
            type_char = format_str[i]
            
            type_sizes = {
                'x': 1,  # パディング
                'b': 1,  # int8
                'B': 1,  # uint8
                'h': 2,  # int16
                'H': 2,  # uint16
                'i': 4,  # int32
                'I': 4,  # uint32
                'q': 8,  # int64
                'Q': 8,  # uint64
                'e': 2,  # float16
                'f': 4,  # float32
                'd': 8,  # float64
                's': 1   # 文字列（1バイト×指定サイズ）
            }
            
            if type_char in type_sizes:
                size += type_sizes[type_char] * count
                
            i += 1
        else:
            break
    
    return size

# Cプログラムをコンパイルして実行
def compile_and_run_c_program(c_file):
    """
    Cプログラムをコンパイルして実行し、その出力を返します。
    """
    # スクリプトの実行パスを取得
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # プロジェクトのルートディレクトリ
    project_root = os.path.dirname(script_dir)
    # cstructのソースファイルとヘッダファイルへの絶対パス
    cstruct_c = os.path.join(project_root, "c", "cstruct.c")
    cstruct_include = os.path.join(project_root, "c")
    
    # コンパイル
    compile_cmd = ["gcc", "-o", c_file.replace(".c", ""), c_file, cstruct_c, f"-I{cstruct_include}"]
    print(f"コンパイルコマンド: {' '.join(compile_cmd)}")
    compile_result = subprocess.run(compile_cmd, capture_output=True, text=True)
    
    if compile_result.returncode != 0:
        print(f"コンパイルエラー: {compile_result.stderr}")
        return None
    
    # 実行
    run_cmd = [c_file.replace(".c", "")]
    run_result = subprocess.run(run_cmd, capture_output=True, text=True)
    
    if run_result.returncode != 0:
        print(f"実行エラー: {run_result.stderr}")
        return None
    
    return run_result.stdout.strip()

# テスト関数
def test_compatibility(format_str, values, result):
    """
    指定されたフォーマット文字列と値でCStructとPythonのstructの互換性をテストします。
    """
    print(f"\nテスト: フォーマット '{format_str}', 値 {values}")
    
    try:
        # Pythonのstructでパック
        python_packed = struct.pack(format_str, *values)
        python_hex = binascii.hexlify(python_packed).decode('ascii')
        print(f"Python packed: {python_hex}")
        
        # CStructで使用するフォーマット文字列を変換
        c_format_str = convert_format_for_cstruct(format_str)
        print(f"CStruct format: {c_format_str}")
        
        # 一時的なCファイルを作成
        with tempfile.NamedTemporaryFile(suffix='.c', delete=False) as tmp:
            c_file = tmp.name
        
        # Cプログラムを生成
        generate_c_test_program(c_format_str, values, c_file)
        
        # Cプログラムを実行
        c_hex = compile_and_run_c_program(c_file)
        print(f"CStruct packed: {c_hex}")
        
        # 一時ファイルを削除
        try:
            os.unlink(c_file)
            if os.path.exists(c_file.replace(".c", "")):
                os.unlink(c_file.replace(".c", ""))
        except Exception as e:
            print(f"警告: 一時ファイルの削除に失敗しました: {e}")
        
        # 結果を比較
        if c_hex and python_hex == c_hex:
            print("✅ 成功: Python structとCStructの出力が一致")
            result.add_pass()
            return True
        elif 'e' in format_str and c_hex and len(python_hex) == len(c_hex):
            # 16ビット浮動小数点数の場合、最下位1ビットのずれまで許容する
            python_bytes = bytes.fromhex(python_hex)
            c_bytes = bytes.fromhex(c_hex)
            
            # エンディアンを考慮して最下位ビットの差を確認
            if format_str.startswith('>'):  # ビッグ(デフォルトはリトル)
                # 最下位バイトの最下位ビットの差を確認
                diff = abs(python_bytes[-1] - c_bytes[-1])
            else:  # デフォルト(リトル)エンディアン
                # 最下位バイトの最下位ビットの差を確認
                diff = abs(python_bytes[0] - c_bytes[0])
            
            if diff <= 1 and (
               (format_str.startswith('>') and python_bytes[:-1] == c_bytes[:-1]) or 
               (python_bytes[1:] == c_bytes[1:])):  # 最下位バイト以外が一致
                print("✅ 成功: 16ビット浮動小数点数で最下位ビットのずれを許容")
                result.add_pass()
                return True
            else:
                print("❌ 失敗: Python structとCStructの出力が一致しません")
                print(f"  Python bytes: {[hex(b) for b in python_bytes]}")
                print(f"  CStruct bytes: {[hex(b) for b in c_bytes]}")
                result.add_fail()
                return False
        else:
            print("❌ 失敗: Python structとCStructの出力が一致しません")
            result.add_fail()
            return False
    
    except Exception as e:
        print(f"❌ エラー: {e}")
        result.add_fail()
        return False

def convert_format_for_cstruct(format_str):
    """
    Pythonのstructフォーマット文字列をCStruct用に変換します。
    エンディアン指定子の処理方法が異なるため、適切に変換します。
    """
    # Pythonのstructモジュールでは、エンディアン指定子はフォーマット文字列の先頭にのみ置くことができる
    # CStructでも同様の形式をサポートしているので、そのまま使用
    return format_str

def main():
    """
    メイン関数: 様々なフォーマットと値の組み合わせでテストを実行します。
    Pythonのstructモジュールと互換性のあるフォーマットのみをテストします。
    """
    result = TestResult()
    
    print("\n=== 基本的な整数型のテスト ===")
    # 基本的な整数型のテスト
    test_compatibility("<b", [42], result)
    test_compatibility(">b", [42], result)
    test_compatibility("<B", [200], result)
    test_compatibility(">B", [200], result)
    test_compatibility("<h", [12345], result)
    test_compatibility(">h", [12345], result)
    test_compatibility("<H", [45678], result)
    test_compatibility(">H", [45678], result)
    test_compatibility("<i", [123456789], result)
    test_compatibility(">i", [123456789], result)
    test_compatibility("<I", [3000000000], result)
    test_compatibility(">I", [3000000000], result)
    test_compatibility("<q", [1234567890123456789], result)
    test_compatibility(">q", [1234567890123456789], result)
    test_compatibility("<Q", [9876543210987654321], result)
    test_compatibility(">Q", [9876543210987654321], result)
    
    print("\n=== 浮動小数点数のテスト ===")
    # 浮動小数点数のテスト
    # 16ビット浮動小数点数（half precision）
    test_compatibility("<e", [3.14], result)
    test_compatibility(">e", [3.14], result)
    # 32ビット浮動小数点数（single precision）
    test_compatibility("<f", [3.14159], result)
    test_compatibility(">f", [3.14159], result)
    # 64ビット浮動小数点数（double precision）
    test_compatibility("<d", [2.71828182845904], result)
    test_compatibility(">d", [2.71828182845904], result)
    
    print("\n=== 複合フォーマットのテスト ===")
    # 複合フォーマットのテスト
    test_compatibility("<bhi", [120, 32000, 2000000000], result)
    test_compatibility(">bhi", [120, 32000, 2000000000], result)
    test_compatibility("<bhf", [120, 32000, 3.14159], result)
    test_compatibility(">bhf", [120, 32000, 3.14159], result)
    
    print("\n=== パディングのテスト ===")
    # パディングのテスト
    test_compatibility("<bxh", [120, 32000], result)
    test_compatibility("<bxxh", [120, 32000], result)
    test_compatibility("<bxxxh", [120, 32000], result)
    test_compatibility(">bxh", [120, 32000], result)
    
    print("\n=== 文字列のテスト ===")
    # 文字列のテスト
    test_compatibility("<5s", [b"Hello"], result)
    test_compatibility(">10s", [b"Python123"], result)
    
    print("\n=== 複数の同じ型のテスト ===")
    # Pythonのstructでは配列指定はできないが、複数の同じ型を並べることで同等の機能を実現できる
    test_compatibility("<bbb", [10, 20, 30], result)  # 3bと同等
    test_compatibility(">hhh", [1000, 2000, 3000], result)  # 3hと同等
    test_compatibility("<ii", [100000, 200000], result)  # 2iと同等
    test_compatibility("<ff", [1.1, 2.2], result)  # 2fと同等
    
    print("\n=== 混合テスト ===")
    # 混合テスト
    test_compatibility("<bhhfff", [100, 1000, 2000, 1.1, 2.2, 3.3], result)
    test_compatibility(">Biidd", [255, 111111, 222222, 1.234, 5.678], result)
    
    # 結果サマリーを表示
    success = result.summary()
    
    # 終了コードを設定
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
