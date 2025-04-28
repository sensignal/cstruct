/* =========================================================================
    cstruct; binary pack/unpack tools.
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

/**
 * @file cstruct.c
 * @brief バイナリデータのパックとアンパックを行うライブラリの実装
 *
 * このライブラリは、構造化されたデータをバイナリ形式に変換（パック）したり、
 * バイナリデータから構造化されたデータに変換（アンパック）したりするための
 * 関数を提供します。
 */
#include "cstruct.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

// エンディアン検出マクロ
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
    #define CSTRUCT_IS_BIG_ENDIAN (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#else
    #define CSTRUCT_IS_BIG_ENDIAN ( \
        (((union { uint16_t u16; uint8_t u8[2]; }){ 0x0100 }).u8[0] == 0x01) )
#endif

/** @brief 10進数の桁数指定における整数オーバーフロー検出のための閾値 */
static const size_t CSTRUCT_DIV10_THRESHOLD = SIZE_MAX / 10;
/** @brief 10進数の桁数指定における整数オーバーフロー検出のための最大最終桁 */
static const size_t CSTRUCT_DIV10_MAX_LAST_DIGIT = SIZE_MAX % 10;

/**
 * @brief バイト順をそのままコピーする（フォワードコピー）
 * @param dst 格納先バッファ
 * @param src 元データ
 * @param size バイトサイズ
 */
static inline void cstruct_store_fw(uint8_t *dst, const void *src, size_t size) {
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < size; ++i) dst[i] = s[i];
}

/**
 * @brief バイト順を逆転してコピーする（リバースコピー）
 * @param dst 格納先バッファ
 * @param src 元データ
 * @param size バイトサイズ
 */
static inline void cstruct_store_rev(uint8_t *dst, const void *src, size_t size) {
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < size; ++i) dst[i] = s[size - 1 - i];
}

/**
 * @brief リトルエンディアン形式でデータを格納する
 * @param dst 格納先バッファ
 * @param src 元データ
 * @param size バイトサイズ
 */
static void cstruct_store_le(uint8_t *dst, const void *src, size_t size) {
#if CSTRUCT_IS_BIG_ENDIAN
    cstruct_store_rev(dst, src, size);  // ビッグエンディアン環境ではバイト順を逆転
#else
    cstruct_store_fw(dst, src, size);   // リトルエンディアン環境ではそのまま
#endif
}

/**
 * @brief ビッグエンディアン形式でデータを格納する
 * @param dst 格納先バッファ
 * @param src 元データ
 * @param size バイトサイズ
 */
static void cstruct_store_be(uint8_t *dst, const void *src, size_t size) {
#if CSTRUCT_IS_BIG_ENDIAN
    cstruct_store_fw(dst, src, size);   // ビッグエンディアン環境ではそのまま
#else
    cstruct_store_rev(dst, src, size);  // リトルエンディアン環境ではバイト順を逆転
#endif
}

/**
 * @brief バイト順をそのままコピーする（フォワードコピー）
 * @param dst 読み出し先バッファ
 * @param src 元データ
 * @param size バイトサイズ
 */
static inline void cstruct_load_fw(void *dst, const uint8_t *src, size_t size) {
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < size; ++i) d[i] = src[i];
}

/**
 * @brief バイト順を逆転してコピーする（リバースコピー）
 * @param dst 読み出し先バッファ
 * @param src 元データ
 * @param size バイトサイズ
 */
static inline void cstruct_load_rev(void *dst, const uint8_t *src, size_t size) {
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < size; ++i) d[i] = src[size - 1 - i];
}

/**
 * @brief リトルエンディアン形式でデータを読み出す
 * @param dst 読み出し先バッファ
 * @param src 元データ
 * @param size バイトサイズ
 */
static void cstruct_load_le(void *dst, const uint8_t *src, size_t size) {
#if CSTRUCT_IS_BIG_ENDIAN
    cstruct_load_rev(dst, src, size);  // ビッグエンディアン環境ではバイト順を逆転
#else
    cstruct_load_fw(dst, src, size);   // リトルエンディアン環境ではそのまま
#endif
}

/**
 * @brief ビッグエンディアン形式でデータを読み出す
 * @param dst 読み出し先バッファ
 * @param src 元データ
 * @param size バイトサイズ
 */
static void cstruct_load_be(void *dst, const uint8_t *src, size_t size) {
#if CSTRUCT_IS_BIG_ENDIAN
    cstruct_load_fw(dst, src, size);   // ビッグエンディアン環境ではそのまま
#else
    cstruct_load_rev(dst, src, size);  // リトルエンディアン環境ではバイト順を逆転
#endif
}

/**
 * @brief IEEE754 float (32ビット)からIEEE754 half precision (16ビット)に変換する
 * 
 * @param f 変換する32ビット浮動小数点数
 * @return uint16_t 16ビット浮動小数点数のビットパターン
 */
static uint16_t cstruct_float_to_half(float f) {
    union {
        uint32_t u;
        float f;
    } in;

    in.f = f;
    uint32_t bits = in.u;

#if CSTRUCT_IS_BIG_ENDIAN
    // ビッグエンディアンの場合、バイト順を入れ替えてリトルエンディアン形式にする
    bits = ((bits & 0xFF) << 24) | ((bits & 0xFF00) << 8) | 
           ((bits & 0xFF0000) >> 8) | ((bits & 0xFF000000) >> 24);
#endif

    uint32_t sign = (bits >> 16) & 0x8000; // 16bit目に符号を持ってくる
    int32_t expo = ((bits >> 23) & 0xFF) - 127 + 15; // exponent再計算
    uint32_t frac = (bits >> 13) & 0x3FF; // 小数部を10bitに縮める

    if (expo <= 0) {
        // 非正規化数またはゼロ
        if (expo < -10) {
            // 完全にゼロ
            return (uint16_t)sign;
        }
        // 非正規化数を作る（暗黙の1をマージ）
        frac = (bits & 0x7FFFFF) | 0x800000; // 先頭に1を付ける
        frac = frac >> (1 - expo);
        frac = (frac + 0x1000) >> 13; // 丸め
        return (uint16_t)(sign | frac);
    } else if (expo >= 0x1F) {
        // InfかNaN
        if ((bits & 0x7FFFFF) != 0) {
            // NaN
            return (uint16_t)(sign | 0x7C00 | ((bits >> 13) & 0x3FF));
        } else {
            // Inf
            return (uint16_t)(sign | 0x7C00);
        }
    } else {
        // 正規化数
        return (uint16_t)(sign | (expo << 10) | frac);
    }
}

/**
 * @brief IEEE754 half precision (16ビット)からIEEE754 float (32ビット)に変換する
 * 
 * @param h 16ビット浮動小数点数のビットパターン
 * @return float 変換された32ビット浮動小数点数
 */
static float cstruct_half_to_float(uint16_t h) {
    uint32_t sign = (h >> 15) & 0x00000001;
    uint32_t expo = (h >> 10) & 0x0000001F;
    uint32_t frac = h & 0x000003FF;

    uint32_t f;

    if (expo == 0) {
        if (frac == 0) {
            // ±0
            f = sign << 31;
        } else {
            // 非正規化数 -> 正規化floatへ
            // 非正規化数を正規化するために、仮数部の上位bitを探す
            while ((frac & 0x00000400) == 0) {
                frac <<= 1;
                expo -= 1;
            }
            expo += 1;
            frac &= 0x000003FF; // 先頭1を消す
            f = (sign << 31) | ((expo + (127 - 15)) << 23) | (frac << 13);
        }
    } else if (expo == 0x1F) {
        // InfまたはNaN
        f = (sign << 31) | (0xFF << 23) | (frac << 13);
    } else {
        // 正規化数
        f = (sign << 31) | ((expo + (127 - 15)) << 23) | (frac << 13);
    }

    union {
        uint32_t u;
        float f;
    } out;

    out.u = f;

#if CSTRUCT_IS_BIG_ENDIAN
    // ビッグエンディアンの場合、バイト順を入れ替えてリトルエンディアン形式にする
    out.u = ((out.u & 0xFF) << 24) | ((out.u & 0xFF00) << 8) | 
            ((out.u & 0xFF0000) >> 8) | ((out.u & 0xFF000000) >> 24);
#endif

    return out.f;
}

/**
 * @brief フォーマット文字列からトークンを解析する
 * @param fmt_in 解析するフォーマット文字列
 * @param tok_out 解析結果を格納する構造体へのポインタ
 * @param current_endian 現在のエンディアン設定
 * @return 解析後の次の位置、エラー時はNULL
 */
static const char *parse_token(const char *fmt_in, cstruct_token_t *tok_out, cstruct_endian_t *current_endian) {
    const char *p = fmt_in;

    while (*p) {
        if (*p == '<') { *current_endian = CSTRUCT_ENDIAN_LITTLE; p++; continue; }
        if (*p == '>') { *current_endian = CSTRUCT_ENDIAN_BIG; p++; continue; }

        tok_out->endian = *current_endian;
        tok_out->size = 0;

        switch (*p) {
            case 'b': tok_out->type = CSTRUCT_TYPE_INT8; tok_out->size = 1; return p + 1;
            case 'B': tok_out->type = CSTRUCT_TYPE_UINT8; tok_out->size = 1; return p + 1;
            case 'h': tok_out->type = CSTRUCT_TYPE_INT16; tok_out->size = 2; return p + 1;
            case 'H': tok_out->type = CSTRUCT_TYPE_UINT16; tok_out->size = 2; return p + 1;
            case 'i': tok_out->type = CSTRUCT_TYPE_INT32; tok_out->size = 4; return p + 1;
            case 'I': tok_out->type = CSTRUCT_TYPE_UINT32; tok_out->size = 4; return p + 1;
            case 'q': tok_out->type = CSTRUCT_TYPE_INT64; tok_out->size = 8; return p + 1;
            case 'Q': tok_out->type = CSTRUCT_TYPE_UINT64; tok_out->size = 8; return p + 1;
            case 't': tok_out->type = CSTRUCT_TYPE_INT128; tok_out->size = 16; return p + 1;
            case 'T': tok_out->type = CSTRUCT_TYPE_UINT128; tok_out->size = 16; return p + 1;
            case 'e': tok_out->type = CSTRUCT_TYPE_FLOAT16; tok_out->size = 2; return p + 1;
            case 'f': tok_out->type = CSTRUCT_TYPE_FLOAT32; tok_out->size = 4; return p + 1;
            case 'd': tok_out->type = CSTRUCT_TYPE_FLOAT64; tok_out->size = 8; return p + 1;
            case 'x': {
                tok_out->type = CSTRUCT_TYPE_PADDING;
                p++;
                size_t val = 0;
                while (*p && isdigit((unsigned char)*p)) {
                    int digit = *p - '0';
                    if (val > CSTRUCT_DIV10_THRESHOLD ||
                        (val == CSTRUCT_DIV10_THRESHOLD && (size_t)digit > CSTRUCT_DIV10_MAX_LAST_DIGIT)) {
                        return NULL;
                    }
                    val = val * 10 + digit;
                    p++;
                }
                if (val == 0) val = 1;
                tok_out->size = val;
                return p;
            }
        }
        // 不正なフォーマット文字の場合はNULLを返す
        return NULL;
    }
    // 文字列終端に達した場合はNULLを返す
    return NULL;
}

/**
 * @brief 型別パック関数 - パディング
 * @param dst 出力先バッファ
 * @param size パディングサイズ
 * @return パック後の次の位置
 */
void *cstruct_pack_padding(const void *dst, size_t size) {
    return (uint8_t *)dst + size;
}

/**
 * @brief 型別パック関数 - 8ビット符号付き整数
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int8(void *dst, int8_t value) {
    uint8_t *out = (uint8_t *)dst;
    *out = (uint8_t)value;
    return out + 1;
}

/**
 * @brief 型別パック関数 - 8ビット符号なし整数
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint8(void *dst, uint8_t value) {
    uint8_t *out = (uint8_t *)dst;
    *out = value;
    return out + 1;
}

/**
 * @brief 型別パック関数 - 16ビット符号付き整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int16_le(void *dst, int16_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, &value, 2);
    return out + 2;
}

/**
 * @brief 型別パック関数 - 16ビット符号付き整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int16_be(void *dst, int16_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, &value, 2);
    return out + 2;
}

/**
 * @brief 型別パック関数 - 16ビット符号なし整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint16_le(void *dst, uint16_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, &value, 2);
    return out + 2;
}

/**
 * @brief 型別パック関数 - 16ビット符号なし整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint16_be(void *dst, uint16_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, &value, 2);
    return out + 2;
}

/**
 * @brief 型別パック関数 - 32ビット符号付き整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int32_le(void *dst, int32_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, &value, 4);
    return out + 4;
}

/**
 * @brief 型別パック関数 - 32ビット符号付き整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int32_be(void *dst, int32_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, &value, 4);
    return out + 4;
}

/**
 * @brief 型別パック関数 - 32ビット符号なし整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint32_le(void *dst, uint32_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, &value, 4);
    return out + 4;
}

/**
 * @brief 型別パック関数 - 32ビット符号なし整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint32_be(void *dst, uint32_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, &value, 4);
    return out + 4;
}

/**
 * @brief 型別パック関数 - 64ビット符号付き整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int64_le(void *dst, int64_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, &value, 8);
    return out + 8;
}

/**
 * @brief 型別パック関数 - 64ビット符号付き整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int64_be(void *dst, int64_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, &value, 8);
    return out + 8;
}

/**
 * @brief 型別パック関数 - 64ビット符号なし整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint64_le(void *dst, uint64_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, &value, 8);
    return out + 8;
}

/**
 * @brief 型別パック関数 - 64ビット符号なし整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint64_be(void *dst, uint64_t value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, &value, 8);
    return out + 8;
}

/**
 * @brief 型別パック関数 - 128ビット符号付き整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値へのポインタ
 * @return パック後の次の位置
 */
void *cstruct_pack_int128_le(void *dst, const void *value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, value, 16);
    return out + 16;
}

/**
 * @brief 型別パック関数 - 128ビット符号付き整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値へのポインタ
 * @return パック後の次の位置
 */
void *cstruct_pack_int128_be(void *dst, const void *value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, value, 16);
    return out + 16;
}

/**
 * @brief 型別パック関数 - 128ビット符号なし整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値へのポインタ
 * @return パック後の次の位置
 */
void *cstruct_pack_uint128_le(void *dst, const void *value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, value, 16);
    return out + 16;
}

/**
 * @brief 型別パック関数 - 128ビット符号なし整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値へのポインタ
 * @return パック後の次の位置
 */
void *cstruct_pack_uint128_be(void *dst, const void *value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, value, 16);
    return out + 16;
}

/**
 * @brief 型別パック関数 - 16ビット浮動小数点数（半精度）（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float16_le(void *dst, float value) {
    uint8_t *out = (uint8_t *)dst;
    uint16_t half = cstruct_float_to_half(value);
    cstruct_store_le(out, &half, 2);
    return out + 2;
}

/**
 * @brief 型別パック関数 - 16ビット浮動小数点数（半精度）（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float16_be(void *dst, float value) {
    uint8_t *out = (uint8_t *)dst;
    uint16_t half = cstruct_float_to_half(value);
    cstruct_store_be(out, &half, 2);
    return out + 2;
}

/**
 * @brief 型別パック関数 - 32ビット浮動小数点数（単精度）（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float32_le(void *dst, float value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, &value, 4);
    return out + 4;
}

/**
 * @brief 型別パック関数 - 32ビット浮動小数点数（単精度）（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float32_be(void *dst, float value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, &value, 4);
    return out + 4;
}

/**
 * @brief 型別パック関数 - 64ビット浮動小数点数（倍精度）（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float64_le(void *dst, double value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_le(out, &value, 8);
    return out + 8;
}

/**
 * @brief 型別パック関数 - 64ビット浮動小数点数（倍精度）（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float64_be(void *dst, double value) {
    uint8_t *out = (uint8_t *)dst;
    cstruct_store_be(out, &value, 8);
    return out + 8;
}

/**
 * @brief 型別アンパック関数 - 8ビット符号付き整数
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int8(const void *src, int8_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    *value = (int8_t)*in;
    return in + 1;
}

/**
 * @brief 型別アンパック関数 - 8ビット符号なし整数
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint8(const void *src, uint8_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    *value = *in;
    return in + 1;
}

/**
 * @brief 型別アンパック関数 - 16ビット符号付き整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int16_le(const void *src, int16_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_le(value, in, 2);
    return in + 2;
}

/**
 * @brief 型別アンパック関数 - 16ビット符号付き整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int16_be(const void *src, int16_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_be(value, in, 2);
    return in + 2;
}

/**
 * @brief 型別アンパック関数 - 16ビット符号なし整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint16_le(const void *src, uint16_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_le(value, in, 2);
    return in + 2;
}

/**
 * @brief 型別アンパック関数 - 16ビット符号なし整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint16_be(const void *src, uint16_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_be(value, in, 2);
    return in + 2;
}

/**
 * @brief 型別アンパック関数 - 32ビット符号付き整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int32_le(const void *src, int32_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_le(value, in, 4);
    return in + 4;
}

/**
 * @brief 型別アンパック関数 - 32ビット符号付き整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int32_be(const void *src, int32_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_be(value, in, 4);
    return in + 4;
}

/**
 * @brief 型別アンパック関数 - 32ビット符号なし整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint32_le(const void *src, uint32_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_le(value, in, 4);
    return in + 4;
}

/**
 * @brief 型別アンパック関数 - 32ビット符号なし整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint32_be(const void *src, uint32_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_be(value, in, 4);
    return in + 4;
}

/**
 * @brief 型別アンパック関数 - 64ビット符号付き整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int64_le(const void *src, int64_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_le(value, in, 8);
    return in + 8;
}

/**
 * @brief 型別アンパック関数 - 64ビット符号付き整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int64_be(const void *src, int64_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_be(value, in, 8);
    return in + 8;
}

/**
 * @brief 型別アンパック関数 - 64ビット符号なし整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint64_le(const void *src, uint64_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_le(value, in, 8);
    return in + 8;
}

/**
 * @brief 型別アンパック関数 - 64ビット符号なし整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint64_be(const void *src, uint64_t *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_be(value, in, 8);
    return in + 8;
}

/**
 * @brief 型別アンパック関数 - 128ビット符号付き整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int128_le(const void *src, void *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_le(value, in, 16);
    return in + 16;
}

/**
 * @brief 型別アンパック関数 - 128ビット符号付き整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int128_be(const void *src, void *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_be(value, in, 16);
    return in + 16;
}

/**
 * @brief 型別アンパック関数 - 128ビット符号なし整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint128_le(const void *src, void *value) {
    return cstruct_unpack_int128_le(src, value);
}

/**
 * @brief 型別アンパック関数 - 128ビット符号なし整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint128_be(const void *src, void *value) {
    return cstruct_unpack_int128_be(src, value);
}

/**
 * @brief 型別アンパック関数 - 16ビット浮動小数点数（半精度）（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float16_le(const void *src, float *value) {
    const uint8_t *in = (const uint8_t *)src;
    uint16_t half;
    cstruct_load_le(&half, in, 2);
    *value = cstruct_half_to_float(half);
    return in + 2;
}

/**
 * @brief 型別アンパック関数 - 16ビット浮動小数点数（半精度）（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float16_be(const void *src, float *value) {
    const uint8_t *in = (const uint8_t *)src;
    uint16_t half;
    cstruct_load_be(&half, in, 2);
    *value = cstruct_half_to_float(half);
    return in + 2;
}

/**
 * @brief 型別アンパック関数 - 32ビット浮動小数点数（単精度）（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float32_le(const void *src, float *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_le(value, in, 4);
    return in + 4;
}

/**
 * @brief 型別アンパック関数 - 32ビット浮動小数点数（単精度）（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float32_be(const void *src, float *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_be(value, in, 4);
    return in + 4;
}

/**
 * @brief 型別アンパック関数 - 64ビット浮動小数点数（倍精度）（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float64_le(const void *src, double *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_le(value, in, 8);
    return in + 8;
}

/**
 * @brief 型別アンパック関数 - 64ビット浮動小数点数（倍精度）（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float64_be(const void *src, double *value) {
    const uint8_t *in = (const uint8_t *)src;
    cstruct_load_be(value, in, 8);
    return in + 8;
}

/**
 * @brief バイナリデータにパックする
 * 
 * 指定されたフォーマット文字列に従って、可変引数のデータをバイナリ形式に変換し、
 * 指定されたバッファに格納します。
 *
 * @param dst 出力先バッファ
 * @param dstlen 出力先バッファのサイズ
 * @param fmt フォーマット文字列
 * @param ... フォーマット文字列に対応する値
 * @return パック後の次の位置、エラー時はNULL
 */
void *cstruct_pack_v(void *dst, size_t dstlen, const char *fmt, va_list args) {
    uint8_t *out = (uint8_t *)dst;
    const uint8_t *end = out + dstlen;
    cstruct_endian_t current_endian = CSTRUCT_ENDIAN_LITTLE; // デフォルトはリトルエンディアン
    
    cstruct_token_t tok;
    const char *next_fmt = fmt;
    while (next_fmt != NULL && *next_fmt != '\0') {
        next_fmt = parse_token(next_fmt, &tok, &current_endian);
        
        if (next_fmt == NULL) {
            // フォーマット文字列の解析エラー
            return NULL;
        }
        
        if ((size_t)(end - out) < tok.size) {
            return NULL;
        }

        switch (tok.type) {
            case CSTRUCT_TYPE_PADDING:
                out = cstruct_pack_padding(out, tok.size);
                break;
            case CSTRUCT_TYPE_FLOAT32: {
                float f = (float)va_arg(args, double);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_float32_le(out, f);
                } else {
                    out = cstruct_pack_float32_be(out, f);
                }
                break;
            }
            case CSTRUCT_TYPE_FLOAT64: {
                double d = va_arg(args, double);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_float64_le(out, d);
                } else {
                    out = cstruct_pack_float64_be(out, d);
                }
                break;
            }
            case CSTRUCT_TYPE_FLOAT16: {
                float f = (float)va_arg(args, double);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_float16_le(out, f);
                } else {
                    out = cstruct_pack_float16_be(out, f);
                }
                break;
            }
            case CSTRUCT_TYPE_INT8: {
                int8_t val = (int8_t)va_arg(args, int);
                out = cstruct_pack_int8(out, val);
                break;
            }
            case CSTRUCT_TYPE_UINT8: {
                uint8_t val = (uint8_t)va_arg(args, int);
                out = cstruct_pack_uint8(out, val);
                break;
            }
            case CSTRUCT_TYPE_INT16: {
                int16_t val = (int16_t)va_arg(args, int);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_int16_le(out, val);
                } else {
                    out = cstruct_pack_int16_be(out, val);
                }
                break;
            }
            case CSTRUCT_TYPE_UINT16: {
                uint16_t val = (uint16_t)va_arg(args, int);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_uint16_le(out, val);
                } else {
                    out = cstruct_pack_uint16_be(out, val);
                }
                break;
            }
            case CSTRUCT_TYPE_INT32: {
                int32_t val = va_arg(args, int32_t);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_int32_le(out, val);
                } else {
                    out = cstruct_pack_int32_be(out, val);
                }
                break;
            }
            case CSTRUCT_TYPE_UINT32: {
                uint32_t val = va_arg(args, uint32_t);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_uint32_le(out, val);
                } else {
                    out = cstruct_pack_uint32_be(out, val);
                }
                break;
            }
            case CSTRUCT_TYPE_INT64: {
                int64_t val = va_arg(args, int64_t);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_int64_le(out, val);
                } else {
                    out = cstruct_pack_int64_be(out, val);
                }
                break;
            }
            case CSTRUCT_TYPE_UINT64: {
                uint64_t val = va_arg(args, uint64_t);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_uint64_le(out, val);
                } else {
                    out = cstruct_pack_uint64_be(out, val);
                }
                break;
            }
            case CSTRUCT_TYPE_INT128: {
                const void *src = va_arg(args, const void *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_int128_le(out, src);
                } else {
                    out = cstruct_pack_int128_be(out, src);
                }
                break;
            }
            case CSTRUCT_TYPE_UINT128: {
                const void *src = va_arg(args, const void *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    out = cstruct_pack_uint128_le(out, src);
                } else {
                    out = cstruct_pack_uint128_be(out, src);
                }
                break;
            }
        }
    }

    return out; // 正常終了時は現在の出力位置を返す
}

void *cstruct_pack(void *dst, size_t dstlen, const char *fmt, ...) {
    void* result;
    va_list args;
    va_start(args, fmt);
    result = cstruct_pack_v(dst, dstlen, fmt, args);
    va_end(args);
    return result;
}

/**
 * @brief バイナリデータからアンパックする（va_list版）
 * 
 * 指定されたフォーマット文字列に従って、バイナリデータを可変引数で指定された
 * 変数にアンパックします。
 *
 * @param src 入力元バッファ
 * @param srclen 入力元バッファのサイズ
 * @param fmt フォーマット文字列
 * @param args 可変引数リスト
 * @return アンパック後の次の位置、エラー時はNULL
 */
const void *cstruct_unpack_v(const void *src, size_t srclen, const char *fmt, va_list args) {
    const uint8_t *in = (const uint8_t *)src;
    const uint8_t *end = in + srclen;
    cstruct_endian_t current_endian = CSTRUCT_ENDIAN_LITTLE; // デフォルトはリトルエンディアン

    cstruct_token_t tok;
    const char *next_fmt = fmt;
    while (next_fmt != NULL && *next_fmt != '\0') {
        next_fmt = parse_token(next_fmt, &tok, &current_endian);
        
        if (next_fmt == NULL) {
            // フォーマット文字列の解析エラー
            return NULL;
        }
        
        if ((size_t)(end - in) < tok.size) {
            return NULL;
        }

        switch (tok.type) {
            case CSTRUCT_TYPE_PADDING:
                in = cstruct_pack_padding(in, tok.size);
                break;
            case CSTRUCT_TYPE_FLOAT32: {
                float *f = va_arg(args, float *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_float32_le(in, f);
                } else {
                    in = cstruct_unpack_float32_be(in, f);
                }
                break;
            }
            case CSTRUCT_TYPE_FLOAT64: {
                double *d = va_arg(args, double *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_float64_le(in, d);
                } else {
                    in = cstruct_unpack_float64_be(in, d);
                }
                break;
            }
            case CSTRUCT_TYPE_FLOAT16: {
                float *f = va_arg(args, float *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_float16_le(in, f);
                } else {
                    in = cstruct_unpack_float16_be(in, f);
                }
                break;
            }
            case CSTRUCT_TYPE_INT8: {
                int8_t *val = va_arg(args, int8_t *);
                in = cstruct_unpack_int8(in, val);
                break;
            }
            case CSTRUCT_TYPE_UINT8: {
                uint8_t *val = va_arg(args, uint8_t *);
                in = cstruct_unpack_uint8(in, val);
                break;
            }
            case CSTRUCT_TYPE_INT16: {
                int16_t *val = va_arg(args, int16_t *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_int16_le(in, val);
                } else {
                    in = cstruct_unpack_int16_be(in, val);
                }
                break;
            }
            case CSTRUCT_TYPE_UINT16: {
                uint16_t *val = va_arg(args, uint16_t *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_uint16_le(in, val);
                } else {
                    in = cstruct_unpack_uint16_be(in, val);
                }
                break;
            }
            case CSTRUCT_TYPE_INT32: {
                int32_t *val = va_arg(args, int32_t *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_int32_le(in, val);
                } else {
                    in = cstruct_unpack_int32_be(in, val);
                }
                break;
            }
            case CSTRUCT_TYPE_UINT32: {
                uint32_t *val = va_arg(args, uint32_t *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_uint32_le(in, val);
                } else {
                    in = cstruct_unpack_uint32_be(in, val);
                }
                break;
            }
            case CSTRUCT_TYPE_INT64: {
                int64_t *val = va_arg(args, int64_t *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_int64_le(in, val);
                } else {
                    in = cstruct_unpack_int64_be(in, val);
                }
                break;
            }
            case CSTRUCT_TYPE_UINT64: {
                uint64_t *val = va_arg(args, uint64_t *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_uint64_le(in, val);
                } else {
                    in = cstruct_unpack_uint64_be(in, val);
                }
                break;
            }
            case CSTRUCT_TYPE_INT128: {
                void *val = va_arg(args, void *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_int128_le(in, val);
                } else {
                    in = cstruct_unpack_int128_be(in, val);
                }
                break;
            }
            case CSTRUCT_TYPE_UINT128: {
                void *val = va_arg(args, void *);
                if (tok.endian == CSTRUCT_ENDIAN_LITTLE) {
                    in = cstruct_unpack_uint128_le(in, val);
                } else {
                    in = cstruct_unpack_uint128_be(in, val);
                }
                break;
            }
        }
    }

    return in; // 正常終了時は現在の入力位置を返す
}

/**
 * @brief バイナリデータからアンパックする
 * 
 * 指定されたフォーマット文字列に従って、バイナリデータを可変引数で指定された
 * 変数にアンパックします。
 *
 * @param src 入力元バッファ
 * @param srclen 入力元バッファのサイズ
 * @param fmt フォーマット文字列
 * @param ... フォーマット文字列に対応する変数へのポインタ
 * @return アンパック後の次の位置、エラー時はNULL
 */
const void *cstruct_unpack(const void *src, size_t srclen, const char *fmt, ...) {
    const void* result;
    va_list args;
    va_start(args, fmt);
    result = cstruct_unpack_v(src, srclen, fmt, args);
    va_end(args);
    return result;
}

/**
 * @brief バイナリデータから特定のフィールドのポインタを取得する
 * 
 * 指定されたフォーマット文字列に従って、バイナリデータ内の特定のフィールドの
 * ポインタを取得します。これにより、バイナリデータを直接操作できます。
 *
 * @param src 入力バイナリデータ
 * @param srclen 入力バイナリデータのサイズ
 * @param fmt フォーマット文字列
 * @param index 取得するフィールドのインデックス（0から始まる）
 * @return フィールドのポインタ、エラー時はNULL
 */
const void *cstruct_get_ptr(const void *src, size_t srclen, const char *fmt, size_t index) {
    const uint8_t *in = (const uint8_t *)src;
    const uint8_t *end = in + srclen;
    cstruct_endian_t current_endian = CSTRUCT_ENDIAN_LITTLE; // デフォルトはリトルエンディアン
    
    size_t current_index = 0;
    cstruct_token_t tok;
    const char *next_fmt = fmt;
    
    while (next_fmt != NULL && *next_fmt != '\0') {
        next_fmt = parse_token(next_fmt, &tok, &current_endian);
        
        if (next_fmt == NULL) {
            // フォーマット文字列の解析エラー
            return NULL;
        }
        
        if ((size_t)(end - in) < tok.size) {
            return NULL;
        }
        
        // パディングも含めてインデックスをカウント
        if (current_index == index) {
            return in;
        }
        current_index++;
        
        in += tok.size;
    }
    
    return NULL; // 指定されたインデックスのフィールドが見つからなかった
}
