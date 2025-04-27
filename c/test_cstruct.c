/* =========================================================================
    cstruct; binary pack/unpack tools.
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

#include "cstruct.h"
#include "unity/src/unity.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

// float値が十分に近いかチェックする関数（浮動小数点の比較用）
static int float_equals(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

// テスト用のグローバル変数
static uint8_t buf[128];
static void *end;
static const void *ret;

// 入力データ
static int8_t   i8;
static uint8_t  u8;
static int16_t  i16;
static uint16_t u16;
static int32_t  i32;
static uint32_t u32;
static int64_t  i64;
static uint64_t u64;
static float    f32;
static double   f64;
static float    f16;
static uint8_t  u128[16];
static int8_t   i128[16];

// アンパック用変数
static int8_t   ri8;
static uint8_t  ru8;
static int16_t  ri16;
static uint16_t ru16;
static int32_t  ri32;
static uint32_t ru32;
static int64_t  ri64;
static uint64_t ru64;
static float    rf16;
static float    rf32;
static double   rf64;
static uint8_t  ru128[16];
static int8_t   ri128[16];

void setUp(void) {
    // 各テスト前に実行される初期化処理
    memset(buf, 0, sizeof(buf));
    
    // 入力データの初期化 - 各データ型の特徴的な値を使用
    i8  = -12;            // 8ビット符号付き整数の一般的な負の値
    u8  = 0x12;           // 8ビット符号なし整数（16進数表記）
    i16 = -1234;          // 16ビット符号付き整数の一般的な負の値
    u16 = 0x3456;         // 16ビット符号なし整数（16進数表記）
    i32 = -12345678;      // 32ビット符号付き整数の一般的な負の値
    u32 = 0x789ABCDE;     // 32ビット符号なし整数（16進数表記）
    i64 = -0x123456789ABCDEFLL;  // 64ビット符号付き整数の大きな負の値
    u64 = 0xFEDCBA9876543210ULL; // 64ビット符号なし整数の大きな値
    f32 = 3.14159f;       // 単精度浮動小数点数（π）
    f64 = 2.7182818284;   // 倍精度浮動小数点数（e）
    f16 = 1.0f;           // 半精度浮動小数点数用のテスト値
    
    for (int i = 0; i < 16; ++i) {
        u128[i] = (uint8_t)i;     // 128ビット符号なし整数（0～15のシーケンス）
        i128[i] = (int8_t)(-i);   // 128ビット符号付き整数（0～-15のシーケンス）
    }
}

void tearDown(void) {
    // 各テスト後に実行されるクリーンアップ処理
    // 特に必要なければ空でも問題ない
}

void test_normal_pack_unpack(void) {
    // 正常パック（エンディアン混在）- パディングを含む
    end = cstruct_pack(buf, sizeof(buf), "bBhHiIqQefdx0x4>Tt",
        i8, u8, i16, u16, i32, u32, i64, u64, f16, f32, f64, u128, i128);
    TEST_ASSERT_NOT_NULL(end);

    // 正常アンパック - パディングを含む
    ret = cstruct_unpack(buf, sizeof(buf), "bBhHiIqQefdx0x4>Tt",
        &ri8, &ru8, &ri16, &ru16, &ri32, &ru32, &ri64, &ru64,
        &rf16, &rf32, &rf64, ru128, ri128);
    TEST_ASSERT_NOT_NULL(ret);

    // 値検証 - パックとアンパック後の値が一致することを確認
    TEST_ASSERT_EQUAL_INT8(i8, ri8);
    TEST_ASSERT_EQUAL_UINT8(u8, ru8);
    TEST_ASSERT_EQUAL_INT16(i16, ri16);
    TEST_ASSERT_EQUAL_UINT16(u16, ru16);
    TEST_ASSERT_EQUAL_INT32(i32, ri32);
    TEST_ASSERT_EQUAL_UINT32(u32, ru32);
    TEST_ASSERT_EQUAL_INT64(i64, ri64);
    TEST_ASSERT_EQUAL_UINT64(u64, ru64);
    
    TEST_ASSERT_TRUE(float_equals(rf16, f16, 0.001f)); // 半精度浮動小数点数は精度が低いため、より大きなイプシロンを使用
    TEST_ASSERT_TRUE(float_equals(rf32, f32, 0.0001f)); // 単精度浮動小数点数の比較
    TEST_ASSERT_TRUE((rf64 - f64) < 0.0000001); // 倍精度浮動小数点数の比較
    
    TEST_ASSERT_EQUAL_INT8_ARRAY(i128, ri128, 16);  // 128ビット符号付き整数配列の比較
    TEST_ASSERT_EQUAL_UINT8_ARRAY(u128, ru128, 16); // 128ビット符号なし整数配列の比較
}

void test_endian_reversed_pack_unpack(void) {
    // 正常パック（エンディアン指定を切り替え）- パディングを含む
    end = cstruct_pack(buf, sizeof(buf), ">bBhHiIqQefdx3x4<Tt",
        i8, u8, i16, u16, i32, u32, i64, u64, f16, f32, f64, u128, i128);
    TEST_ASSERT_NOT_NULL(end);

    // 正常アンパック - エンディアン指定を切り替え
    ret = cstruct_unpack(buf, sizeof(buf), ">bBhHiIqQefdx3x4<Tt",
        &ri8, &ru8, &ri16, &ru16, &ri32, &ru32, &ri64, &ru64,
        &rf16, &rf32, &rf64, ru128, ri128);
    TEST_ASSERT_NOT_NULL(ret);

    // 値検証 - エンディアンが切り替わっても値が正しく復元されることを確認
    TEST_ASSERT_EQUAL_INT8(i8, ri8);
    TEST_ASSERT_EQUAL_UINT8(u8, ru8);
    TEST_ASSERT_EQUAL_INT16(i16, ri16);
    TEST_ASSERT_EQUAL_UINT16(u16, ru16);
    TEST_ASSERT_EQUAL_INT32(i32, ri32);
    TEST_ASSERT_EQUAL_UINT32(u32, ru32);
    TEST_ASSERT_EQUAL_INT64(i64, ri64);
    TEST_ASSERT_EQUAL_UINT64(u64, ru64);
    
    TEST_ASSERT_TRUE(float_equals(rf16, f16, 0.001f)); // 半精度浮動小数点数は精度が低いため、より大きなイプシロンを使用
    TEST_ASSERT_TRUE(float_equals(rf32, f32, 0.0001f)); // 単精度浮動小数点数の比較
    TEST_ASSERT_TRUE((rf64 - f64) < 0.0000001); // 倍精度浮動小数点数の比較
    
    TEST_ASSERT_EQUAL_INT8_ARRAY(i128, ri128, 16);  // 128ビット符号付き整数配列の比較
    TEST_ASSERT_EQUAL_UINT8_ARRAY(u128, ru128, 16); // 128ビット符号なし整数配列の比較
}

void test_half_precision(void) {
    // 半精度浮動小数点数（IEEE754 16ビット）の詳細テスト
    float f16_values[] = {
        0.0f,       // ゼロ
        -0.0f,      // 負のゼロ（符号ビットが1）
        1.0f,       // 1.0（標準的な正規化数）
        -1.0f,      // -1.0（負の正規化数）
        0.5f,       // 0.5（指数部が-1の正規化数）
        -0.5f,      // -0.5（負の指数部が-1の正規化数）
        65504.0f,   // 半精度で表現可能な最大の正規化数
        -65504.0f,  // 半精度で表現可能な最小の正規化数
        5.96e-8f,   // 半精度で表現可能な最小の正規化数（指数部が-14）
        -5.96e-8f   // 半精度で表現可能な最小の負の正規化数
    };
    
    int num_f16_values = sizeof(f16_values) / sizeof(f16_values[0]);
    
    for (int i = 0; i < num_f16_values; i++) {
        float f16_in = f16_values[i];
        float f16_out;
        
        // パック
        end = cstruct_pack(buf, sizeof(buf), "e", f16_in);
        TEST_ASSERT_NOT_NULL(end);
        
        // アンパック
        ret = cstruct_unpack(buf, sizeof(buf), "e", &f16_out);
        TEST_ASSERT_NOT_NULL(ret);
        
        // 値検証（半精度は精度が低いため、より大きなイプシロンを使用）
        TEST_ASSERT_TRUE(float_equals(f16_out, f16_in, 0.001f));
    }
}

void test_half_precision_endian(void) {
    // 半精度浮動小数点数のエンディアンテスト
    float f16_test = 1.0f;  // 半精度で正確に表現できる値
    float f16_le, f16_be;
    
    // リトルエンディアンでパック
    end = cstruct_pack(buf, sizeof(buf), "<e", f16_test);
    TEST_ASSERT_NOT_NULL(end);
    
    // ビッグエンディアンでパック
    end = cstruct_pack(buf + 2, sizeof(buf) - 2, ">e", f16_test);
    TEST_ASSERT_NOT_NULL(end);
    
    // リトルエンディアンでアンパック
    ret = cstruct_unpack(buf, sizeof(buf), "<e", &f16_le);
    TEST_ASSERT_NOT_NULL(ret);
    
    // ビッグエンディアンでアンパック
    ret = cstruct_unpack(buf + 2, sizeof(buf) - 2, ">e", &f16_be);
    TEST_ASSERT_NOT_NULL(ret);
    
    // 値検証 - エンディアンに関わらず正しく変換されることを確認
    TEST_ASSERT_TRUE(float_equals(f16_le, f16_test, 0.001f));
    TEST_ASSERT_TRUE(float_equals(f16_be, f16_test, 0.001f));
}

void test_padding(void) {
    // パディングテスト
    printf("パディングテストを実行中...\n");
    
    // 0xff埋め
    memset(buf, 0xFF, sizeof(buf));
    
    // "x4" - 4バイトのパディング
    end = cstruct_pack(buf, sizeof(buf), "Ix4I", u32, u32);
    TEST_ASSERT_NOT_NULL(end);
    
    // アンパック
    ret = cstruct_unpack(buf, sizeof(buf), "Ix4I", &ru32, &ru32);
    TEST_ASSERT_NOT_NULL(ret);
    
    // パディング部分が無変更であることを確認
    for (int i = 4; i < 8; i++) {
        TEST_ASSERT_EQUAL_UINT8(0xff, buf[i]);
    }
    
    // 値が正しく復元されていることを確認
    TEST_ASSERT_EQUAL_UINT32(u32, ru32);
}

void test_large_padding(void) {
    // 大きなパディングのテスト
    printf("大きなパディングテストを実行中...\n");
    
    // 0xff埋め
    memset(buf, 0xFF, sizeof(buf));
    
    // "x100" - 100バイトのパディング
    end = cstruct_pack(buf, sizeof(buf), "x100");
    TEST_ASSERT_NOT_NULL(end);
    
    // パディング部分が無変更であることを確認
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_UINT8(0xff, buf[i]);
    }
}

void test_multiple_padding(void) {
    // 複数パディングのテスト
    printf("複数パディングテストを実行中...\n");
    
    // 0xff埋め
    memset(buf, 0xFF, sizeof(buf));
    
    // "x2x3x4" - 連続した複数のパディング（合計9バイト）
    end = cstruct_pack(buf, sizeof(buf), "x2x3x4");
    TEST_ASSERT_NOT_NULL(end);
    
    // パディング部分が無変更であることを確認
    for (int i = 0; i < 9; i++) {
        TEST_ASSERT_EQUAL_UINT8(0xff, buf[i]);
    }
    
    // アンパック
    ret = cstruct_unpack(buf, sizeof(buf), "x2x3x4");
    TEST_ASSERT_NOT_NULL(ret);
}

void test_padding_only(void) {
    // パディングのみのフォーマット文字列テスト
    uint8_t test_buf[32];
    memset(test_buf, 0xFF, sizeof(test_buf));
    
    // "x16" - 16バイトのパディングのみ
    end = cstruct_pack(test_buf, sizeof(test_buf), "x16");
    TEST_ASSERT_NOT_NULL(end);
    
    // パディング部分が無変更であることを確認（0xFFのまま）
    for (int i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL_UINT8(0xff, test_buf[i]);
    }
    
    // パディングのみのフォーマットでアンパック
    ret = cstruct_unpack(test_buf, sizeof(test_buf), "x16");
    TEST_ASSERT_NOT_NULL(ret);
}

void test_buffer_insufficient(void) {
    // バッファ不足時のエラー処理テスト
    // アンパック時のバッファ不足
    ret = cstruct_unpack(buf, 1, "I", &ru32);
    TEST_ASSERT_NULL(ret);

    // パック時のバッファ不足
    ret = cstruct_pack(buf, 1, "I", u32);
    TEST_ASSERT_NULL(ret);
}

void test_get_ptr(void) {
    // cstruct_get_ptr関数のテスト（特定フィールドへのポインタ取得）
    printf("cstruct_get_ptr関数をテスト中...\n");

    // 0xff埋め
    memset(buf, 0xFF, sizeof(buf));
    
    // テスト用データをパック - パディングを含む
    end = cstruct_pack(buf, sizeof(buf), "bBhHiIqQefdx4",
        i8, u8, i16, u16, i32, u32, i64, u64, f16, f32, f64);
    TEST_ASSERT_NOT_NULL(end);
    
    // 各フィールドのポインタを取得して検証
    
    // INT8フィールド（インデックス0）
    const void *field_ptr = cstruct_get_ptr(buf, sizeof(buf), "bBhHiIqQefdx4", 0);
    TEST_ASSERT_NOT_NULL(field_ptr);
    TEST_ASSERT_EQUAL_INT8(i8, *(const int8_t *)field_ptr);
    
    // UINT8フィールド（インデックス1）
    field_ptr = cstruct_get_ptr(buf, sizeof(buf), "bBhHiIqQefdx4", 1);
    TEST_ASSERT_NOT_NULL(field_ptr);
    TEST_ASSERT_EQUAL_UINT8(u8, *(const uint8_t *)field_ptr);
    
    // INT16フィールド（インデックス2）
    field_ptr = cstruct_get_ptr(buf, sizeof(buf), "bBhHiIqQefdx4", 2);
    TEST_ASSERT_NOT_NULL(field_ptr);
    // エンディアンの問題があるため、直接比較はしない
    
    // パディングフィールド - パディングはカウントされないため、インデックスを調整
    field_ptr = cstruct_get_ptr(buf, sizeof(buf), "bBhHiIqQefdx4", 11);
    TEST_ASSERT_NOT_NULL(field_ptr);
    // パディングは無変化のはず（0xFFのまま）
    const uint8_t *padding = (const uint8_t *)field_ptr;
    for (size_t i = 0; i < 4; i++) {  // パディングサイズは4バイト
        TEST_ASSERT_EQUAL_UINT8(0xff, padding[i]);
    }
    
    // 存在しないフィールドインデックス
    field_ptr = cstruct_get_ptr(buf, sizeof(buf), "bBhHiIqQefdx4", 100);
    TEST_ASSERT_NULL(field_ptr);
    
    // バッファサイズ不足
    field_ptr = cstruct_get_ptr(buf, 1, "iI", 1);
    TEST_ASSERT_NULL(field_ptr);
}

void test_ieee754_special_values(void) {
    // IEEE754特殊値のテスト（半精度浮動小数点数）
    printf("IEEE754特殊値をテスト中...\n");
    
    // 無限大
    float inf = INFINITY;
    float neg_inf = -INFINITY;
    
    // NaN（非数）
    float nan_val = NAN;
    
    // 正の無限大のテスト
    end = cstruct_pack(buf, sizeof(buf), "e", inf);
    TEST_ASSERT_NOT_NULL(end);
    ret = cstruct_unpack(buf, sizeof(buf), "e", &rf16);
    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_TRUE(isinf(rf16) && rf16 > 0);
    
    // 負の無限大のテスト
    end = cstruct_pack(buf, sizeof(buf), "e", neg_inf);
    TEST_ASSERT_NOT_NULL(end);
    ret = cstruct_unpack(buf, sizeof(buf), "e", &rf16);
    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_TRUE(isinf(rf16) && rf16 < 0);
    
    // NaN（非数）のテスト
    end = cstruct_pack(buf, sizeof(buf), "e", nan_val);
    TEST_ASSERT_NOT_NULL(end);
    ret = cstruct_unpack(buf, sizeof(buf), "e", &rf16);
    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_TRUE(isnan(rf16));
}

void test_denormalized_values(void) {
    // 非正規化数のテスト（半精度浮動小数点数）
    printf("非正規化数をテスト中...\n");
    
    // 手動で非正規化数のビットパターンを作成
    uint16_t denorm_bits[] = {0x0001, 0x0200}; // 最小の正の非正規化数と別の非正規化数
    
    for (int i = 0; i < 2; i++) {
        // リトルエンディアンでバッファに格納
        memcpy(buf, &denorm_bits[i], 2);
        
        // リトルエンディアンでアンパック
        ret = cstruct_unpack(buf, sizeof(buf), "<e", &rf16);
        TEST_ASSERT_NOT_NULL(ret);
        
        // 非正規化数が正しくアンパックされたことを確認（正の値）
        TEST_ASSERT_TRUE(rf16 > 0.0f);
    }
}

void test_128bit_integer_endianness(void) {
    // 128ビット整数のエンディアンテスト
    printf("128ビット整数のエンディアンをテスト中...\n");
    
    // テスト用の128ビット整数データを準備（0～15のシーケンス）
    uint8_t test_u128[16];
    for (int i = 0; i < 16; ++i) test_u128[i] = i;
    
    // リトルエンディアンでパック
    end = cstruct_pack(buf, sizeof(buf), "<t", test_u128);
    TEST_ASSERT_NOT_NULL(end);
    
    // リトルエンディアンでアンパック
    uint8_t result_le[16] = {0};
    ret = cstruct_unpack(buf, sizeof(buf), "<t", result_le);
    TEST_ASSERT_NOT_NULL(ret);
    
    // リトルエンディアンの場合、元のデータと同じバイト順になるはず
    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_u128, result_le, 16);
    
    // ビッグエンディアンでパック
    end = cstruct_pack(buf, sizeof(buf), ">t", test_u128);
    TEST_ASSERT_NOT_NULL(end);
    
    // ビッグエンディアンでアンパック
    uint8_t result_be[16] = {0};
    ret = cstruct_unpack(buf, sizeof(buf), ">t", result_be);
    TEST_ASSERT_NOT_NULL(ret);
    
    // ビッグエンディアンでパック・アンパックした場合も、
    // 結果は元のデータと同じになるはず（内部でバイト順の変換が行われるため）
    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_u128, result_be, 16);
    
    // バッファの内容を直接検証
    // リトルエンディアンでパックした場合、バッファには元のデータがそのまま格納されるはず
    end = cstruct_pack(buf, sizeof(buf), "<t", test_u128);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_u128, buf, 16);
    
    // ビッグエンディアンでパックした場合、バッファには元のデータが逆順で格納されるはず
    end = cstruct_pack(buf, sizeof(buf), ">t", test_u128);
    for (int i = 0; i < 16; ++i) {
        TEST_ASSERT_EQUAL_UINT8(test_u128[15 - i], buf[i]);
    }
}

void test_format_string_parsing_errors(void) {
    // フォーマット文字列の解析エラーテスト
    printf("フォーマット文字列の解析エラーをテスト中...\n");
    
    // 不正なフォーマット文字
    // 'Z'は定義されていないフォーマット文字
    ret = cstruct_pack(buf, sizeof(buf), "Z", 0);
    TEST_ASSERT_NULL(ret);
    
    // 数値オーバーフロー（非常に大きな数値）
    // SIZE_MAXに近い大きな数値を生成するフォーマット文字列
    char huge_format[100] = "x999999999999999999999";
    ret = cstruct_pack(buf, sizeof(buf), huge_format);
    TEST_ASSERT_NULL(ret);
    
    // cstruct_unpackのフォーマット文字列解析エラーテスト
    ret = cstruct_unpack(buf, sizeof(buf), "Z", &ri8);
    TEST_ASSERT_NULL(ret);
    
    // バッファサイズ不足のテスト
    ret = cstruct_unpack(buf, 1, "iI", &ri32, &ru32);
    TEST_ASSERT_NULL(ret);
}

void test_get_ptr_error_handling(void) {
    // cstruct_get_ptrのエラー処理テスト
    printf("cstruct_get_ptrのエラー処理をテスト中...\n");
    // 不正なフォーマット文字でのcstruct_get_ptr
    const void *ptr = cstruct_get_ptr(buf, sizeof(buf), "Z", 0);
    TEST_ASSERT_NULL(ptr);
    
    // バッファサイズ不足でのcstruct_get_ptr
    ptr = cstruct_get_ptr(buf, 1, "iI", 1);
    TEST_ASSERT_NULL(ptr);
    
    // 存在しないインデックスでのcstruct_get_ptr
    ptr = cstruct_get_ptr(buf, sizeof(buf), "bB", 5);
    TEST_ASSERT_NULL(ptr);
}

void test_empty_format_string(void) {
    // 空のフォーマット文字列と文字列終端のテスト
    
    // 空のフォーマット文字列（有効なケース）
    ret = cstruct_pack(buf, sizeof(buf), "");
    TEST_ASSERT_NOT_NULL(ret); // 空のフォーマット文字列は有効
    
    ret = cstruct_unpack(buf, sizeof(buf), "");
    TEST_ASSERT_NOT_NULL(ret); // 空のフォーマット文字列は有効
    
    // 不完全なフォーマット指定子のテスト
    // エンディアン指定子のみ（後に何もない）
    ret = cstruct_pack(buf, sizeof(buf), "<");
    TEST_ASSERT_NULL(ret); // エンディアン指定子の後に何もないのでNULLを返すはず
    
    // パディング指定子の後に数字がない場合
    ret = cstruct_pack(buf, sizeof(buf), "x");
    TEST_ASSERT_NOT_NULL(ret); // xの後に数字がない場合は1バイトのパディングとして扱われる
}

void test_argument_count(void) {
    // 引数とフォーマット文字の個数が異なる場合のテスト
    printf("引数とフォーマット文字の個数が異なる場合のテスト...\n");
    
    // 注意: 現在の実装では引数の数チェックを行わないため、
    // 引数が足りない場合や多すぎる場合のテストは行わない
    // C言語の可変引数の仕様上、これらを安全にテストすることは難しい
    
    // 代わりに、正常系のテストを再度実行して確認
    ret = cstruct_pack(buf, sizeof(buf), "I", u32);
    TEST_ASSERT_NOT_NULL(ret);
    
    ret = cstruct_unpack(buf, sizeof(buf), "I", &ru32);
    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_EQUAL_UINT32(u32, ru32);
}

void test_half_precision_edge_cases(void) {
    // 半精度浮動小数点数のエッジケーステスト
    printf("半精度浮動小数点数のエッジケースをテスト中...\n");
    
    // 非常に大きな値（半精度の範囲外）
    float huge_float = 65536.0f; // 半精度の最大値（65504）を超える
    end = cstruct_pack(buf, sizeof(buf), "e", huge_float);
    TEST_ASSERT_NOT_NULL(end);
    ret = cstruct_unpack(buf, sizeof(buf), "e", &rf16);
    TEST_ASSERT_NOT_NULL(ret);
    // 大きすぎる値は無限大に丸められるはず
    TEST_ASSERT_TRUE(isinf(rf16) && rf16 > 0);
    
    // 非常に小さな値（半精度の非正規化数として表現可能）
    // 指数部が-24 < exponent < -14 の範囲の値
    float tiny_float = 5.96e-8f * 0.1f; // 非正規化数の範囲内
    end = cstruct_pack(buf, sizeof(buf), "e", tiny_float);
    TEST_ASSERT_NOT_NULL(end);
    ret = cstruct_unpack(buf, sizeof(buf), "e", &rf16);
    TEST_ASSERT_NOT_NULL(ret);
    // 非常に小さな値は非正規化数または0に丸められるはず
    TEST_ASSERT_TRUE(fabsf(rf16) < 6.0e-8f);
    
    // 極めて小さな値（半精度では表現不可能）
    float extremely_tiny = FLT_MIN * 0.0000001f; // 半精度の最小値よりもはるかに小さい
    end = cstruct_pack(buf, sizeof(buf), "e", extremely_tiny);
    TEST_ASSERT_NOT_NULL(end);
    ret = cstruct_unpack(buf, sizeof(buf), "e", &rf16);
    TEST_ASSERT_NOT_NULL(ret);
    // 極めて小さな値は0に丸められるはず
    TEST_ASSERT_TRUE(fabsf(rf16) < 1.0e-10f);
}

void test_denormalized_large_mantissa(void) {
    // 非正規化数で仮数部が大きい値のテスト
    // 指数部が-15で、仮数部が0x3FFを超える値を作成
    // 2^(-15) * (1 + 0.999...) に近い値
    
    // 仮数部が大きい非正規化数を作成（ビット操作で直接作成）
    uint32_t large_bits = 0x37FFFFFF; // 指数部=-16, 仮数部=0x7FFFFF（最大値）
    float large_denorm;
    memcpy(&large_denorm, &large_bits, sizeof(large_denorm));
            
    // パックする
    end = cstruct_pack(buf, sizeof(buf), "e", large_denorm);
    TEST_ASSERT_NOT_NULL(end);
    
    // バッファの内容を直接検証
    uint16_t half_bits;
    memcpy(&half_bits, buf, sizeof(half_bits));
    
    // IEEE754の仕様では、非正規化数の指数部は0
    uint16_t half_exponent = (half_bits >> 10) & 0x1F;
    TEST_ASSERT_EQUAL_UINT16(0, half_exponent);  // 非正規化数の指数部は0
    
    uint16_t half_mantissa = half_bits & 0x3FF;
    TEST_ASSERT_EQUAL_UINT16(0x200, half_mantissa);
}

// メイン関数
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_normal_pack_unpack);
    RUN_TEST(test_endian_reversed_pack_unpack);
    RUN_TEST(test_half_precision);
    RUN_TEST(test_half_precision_endian);
    RUN_TEST(test_padding);
    RUN_TEST(test_large_padding);
    RUN_TEST(test_multiple_padding);
    RUN_TEST(test_padding_only);
    RUN_TEST(test_buffer_insufficient);
    RUN_TEST(test_get_ptr);
    RUN_TEST(test_ieee754_special_values);
    RUN_TEST(test_denormalized_values);
    RUN_TEST(test_128bit_integer_endianness);
    RUN_TEST(test_format_string_parsing_errors);
    RUN_TEST(test_get_ptr_error_handling);
    RUN_TEST(test_empty_format_string);
    RUN_TEST(test_argument_count);
    RUN_TEST(test_half_precision_edge_cases);
    RUN_TEST(test_denormalized_large_mantissa);
    
    return UNITY_END();
}
