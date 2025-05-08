/* =========================================================================
    cstruct; binary pack/unpack tools.
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

/**
 * @file cstruct.h
 * @brief バイナリデータのパックとアンパックを行うライブラリのヘッダファイル
 *
 * このライブラリは、構造化されたデータをバイナリ形式に変換（パック）したり、
 * バイナリデータから構造化されたデータに変換（アンパック）したりするための
 * 関数を提供します。
 *
 * # エンディアン指定
 * 記号    説明
 * <       リトルエンディアン (Little Endian) 指定
 * >       ビッグエンディアン (Big Endian) 指定
 * フォーマット文字列のどこでも切り替え可能
 * エンディアン指定が現れた時点以降に適用される
 * デフォルト（最初）はリトルエンディアン
 *
 * # データ型指定
 * 記号    型          サイズ (bytes)    備考
 * b       int8_t      1                 signed 8bit整数
 * B       uint8_t     1                 unsigned 8bit整数
 * h       int16_t     2                 signed 16bit整数
 * H       uint16_t    2                 unsigned 16bit整数
 * i       int32_t     4                 signed 32bit整数
 * I       uint32_t    4                 unsigned 32bit整数
 * q       int64_t     8                 signed 64bit整数
 * Q       uint64_t    8                 unsigned 64bit整数
 * t       int128_t    16                signed 128bit整数
 * T       uint128_t   16                unsigned 128bit整数
 * e       float       2                 IEEE754 half precision (16ビット浮動小数点数)
 * f       float       4                 IEEE754 float32 (32ビット浮動小数点数)
 * d       double      8                 IEEE754 float64 (64ビット浮動小数点数)
 *
 * # 特別なフィールド
 * 記号    型          サイズ            備考
 * xN      パディング   N bytes          Nバイトのゼロ埋めパディング
 * xN：値は常に0x00で埋められる
 * Nは10進数で桁数指定（例: x3など）
 */
#ifndef CSTRUCT_H
#define CSTRUCT_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief バイナリフォーマットのデータ型
 */
typedef enum {
    CSTRUCT_TYPE_INT8,     /**< 符号付き8ビット整数 */
    CSTRUCT_TYPE_UINT8,    /**< 符号なし8ビット整数 */
    CSTRUCT_TYPE_INT16,    /**< 符号付き16ビット整数 */
    CSTRUCT_TYPE_UINT16,   /**< 符号なし16ビット整数 */
    CSTRUCT_TYPE_INT32,    /**< 符号付き32ビット整数 */
    CSTRUCT_TYPE_UINT32,   /**< 符号なし32ビット整数 */
    CSTRUCT_TYPE_INT64,    /**< 符号付き64ビット整数 */
    CSTRUCT_TYPE_UINT64,   /**< 符号なし64ビット整数 */
    CSTRUCT_TYPE_INT128,   /**< 符号付き128ビット整数 */
    CSTRUCT_TYPE_UINT128,  /**< 符号なし128ビット整数 */
    CSTRUCT_TYPE_FLOAT16,  /**< 16ビット浮動小数点数 (IEEE754 half precision) */
    CSTRUCT_TYPE_FLOAT32,  /**< 32ビット浮動小数点数 (IEEE754 single precision) */
    CSTRUCT_TYPE_FLOAT64,  /**< 64ビット浮動小数点数 (IEEE754 double precision) */
    CSTRUCT_TYPE_PADDING,  /**< パディング（0埋め） */
    CSTRUCT_TYPE_STRING    /**< 文字列 */
} cstruct_type_t;

/**
 * @brief エンディアン指定
 */
typedef enum {
    CSTRUCT_ENDIAN_LITTLE, /**< リトルエンディアン */
    CSTRUCT_ENDIAN_BIG     /**< ビッグエンディアン */
} cstruct_endian_t;

/**
 * @brief フォーマットトークン
 * 
 * フォーマット文字列の各要素を表す構造体
 */
typedef struct {
    cstruct_type_t type;    /**< データ型 */
    cstruct_endian_t endian; /**< エンディアン */
    size_t size;           /**< サイズ（バイト数） */
    size_t count;          /**< 繰り返し回数 */
} cstruct_token_t;

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
void *cstruct_pack(void *dst, size_t dstlen, const char *fmt, ...);

/**
 * @brief バイナリデータにパックする（va_list版）
 * 
 * 指定されたフォーマット文字列に従って、可変引数のデータをバイナリ形式に変換し、
 * 指定されたバッファに格納します。
 *
 * @param dst 出力先バッファ
 * @param dstlen 出力先バッファのサイズ
 * @param fmt フォーマット文字列
 * @param args 可変引数リスト
 * @return パック後の次の位置、エラー時はNULL
 */
void *cstruct_pack_v(void *dst, size_t dstlen, const char *fmt, va_list args);

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
const void *cstruct_unpack(const void *src, size_t srclen, const char *fmt, ...);

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
const void *cstruct_unpack_v(const void *src, size_t srclen, const char *fmt, va_list args);

/**
 * @brief 指定されたインデックスのフィールドの位置を取得する
 * 
 * @param src 入力元バッファ
 * @param srclen 入力元バッファのサイズ
 * @param fmt フォーマット文字列
 * @param index 取得するフィールドのインデックス（0から始まる）
 * @return フィールドの位置へのポインタ、エラー時はNULL
 */
const void *cstruct_get_ptr(const void *src, size_t srclen, const char *fmt, size_t index);

/**
 * @brief 型別パック関数 - パディング
 * @param dst 出力先バッファ
 * @param size パディングサイズ
 * @return パック後の次の位置
 */
void *cstruct_pack_padding(void *dst, size_t size);

/**
 * @brief 型別パック関数 - 8ビット符号付き整数
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int8(void *dst, int8_t value);

/**
 * @brief 型別パック関数 - 8ビット符号なし整数
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint8(void *dst, uint8_t value);

/**
 * @brief 型別パック関数 - 16ビット符号付き整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int16_le(void *dst, int16_t value);

/**
 * @brief 型別パック関数 - 16ビット符号付き整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int16_be(void *dst, int16_t value);

/**
 * @brief 型別パック関数 - 16ビット符号なし整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint16_le(void *dst, uint16_t value);

/**
 * @brief 型別パック関数 - 16ビット符号なし整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint16_be(void *dst, uint16_t value);

/**
 * @brief 型別パック関数 - 32ビット符号付き整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int32_le(void *dst, int32_t value);

/**
 * @brief 型別パック関数 - 32ビット符号付き整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int32_be(void *dst, int32_t value);

/**
 * @brief 型別パック関数 - 32ビット符号なし整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint32_le(void *dst, uint32_t value);

/**
 * @brief 型別パック関数 - 32ビット符号なし整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint32_be(void *dst, uint32_t value);

/**
 * @brief 型別パック関数 - 64ビット符号付き整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int64_le(void *dst, int64_t value);

/**
 * @brief 型別パック関数 - 64ビット符号付き整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_int64_be(void *dst, int64_t value);

/**
 * @brief 型別パック関数 - 64ビット符号なし整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint64_le(void *dst, uint64_t value);

/**
 * @brief 型別パック関数 - 64ビット符号なし整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_uint64_be(void *dst, uint64_t value);

/**
 * @brief 型別パック関数 - 128ビット符号付き整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値へのポインタ
 * @return パック後の次の位置
 */
void *cstruct_pack_int128_le(void *dst, const void *value);

/**
 * @brief 型別パック関数 - 128ビット符号付き整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値へのポインタ
 * @return パック後の次の位置
 */
void *cstruct_pack_int128_be(void *dst, const void *value);

/**
 * @brief 型別パック関数 - 128ビット符号なし整数（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値へのポインタ
 * @return パック後の次の位置
 */
void *cstruct_pack_uint128_le(void *dst, const void *value);

/**
 * @brief 型別パック関数 - 128ビット符号なし整数（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値へのポインタ
 * @return パック後の次の位置
 */
void *cstruct_pack_uint128_be(void *dst, const void *value);

/**
 * @brief 型別パック関数 - 16ビット浮動小数点数（半精度）（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float16_le(void *dst, float value);

/**
 * @brief 型別パック関数 - 16ビット浮動小数点数（半精度）（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float16_be(void *dst, float value);

/**
 * @brief 型別パック関数 - 32ビット浮動小数点数（単精度）（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float32_le(void *dst, float value);

/**
 * @brief 型別パック関数 - 32ビット浮動小数点数（単精度）（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float32_be(void *dst, float value);

/**
 * @brief 型別パック関数 - 64ビット浮動小数点数（倍精度）（リトルエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float64_le(void *dst, double value);

/**
 * @brief 型別パック関数 - 64ビット浮動小数点数（倍精度）（ビッグエンディアン）
 * @param dst 出力先バッファ
 * @param value パックする値
 * @return パック後の次の位置
 */
void *cstruct_pack_float64_be(void *dst, double value);

/**
 * @brief 型別アンパック関数 - 8ビット符号付き整数
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int8(const void *src, int8_t *value);

/**
 * @brief 型別アンパック関数 - 8ビット符号なし整数
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint8(const void *src, uint8_t *value);

/**
 * @brief 型別アンパック関数 - 16ビット符号付き整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int16_le(const void *src, int16_t *value);

/**
 * @brief 型別アンパック関数 - 16ビット符号付き整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int16_be(const void *src, int16_t *value);

/**
 * @brief 型別アンパック関数 - 16ビット符号なし整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint16_le(const void *src, uint16_t *value);

/**
 * @brief 型別アンパック関数 - 16ビット符号なし整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint16_be(const void *src, uint16_t *value);

/**
 * @brief 型別アンパック関数 - 32ビット符号付き整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int32_le(const void *src, int32_t *value);

/**
 * @brief 型別アンパック関数 - 32ビット符号付き整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int32_be(const void *src, int32_t *value);

/**
 * @brief 型別アンパック関数 - 32ビット符号なし整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint32_le(const void *src, uint32_t *value);

/**
 * @brief 型別アンパック関数 - 32ビット符号なし整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint32_be(const void *src, uint32_t *value);

/**
 * @brief 型別アンパック関数 - 64ビット符号付き整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int64_le(const void *src, int64_t *value);

/**
 * @brief 型別アンパック関数 - 64ビット符号付き整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int64_be(const void *src, int64_t *value);

/**
 * @brief 型別アンパック関数 - 64ビット符号なし整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint64_le(const void *src, uint64_t *value);

/**
 * @brief 型別アンパック関数 - 64ビット符号なし整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint64_be(const void *src, uint64_t *value);

/**
 * @brief 型別アンパック関数 - 128ビット符号付き整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int128_le(const void *src, void *value);

/**
 * @brief 型別アンパック関数 - 128ビット符号付き整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_int128_be(const void *src, void *value);

/**
 * @brief 型別アンパック関数 - 128ビット符号なし整数（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint128_le(const void *src, void *value);

/**
 * @brief 型別アンパック関数 - 128ビット符号なし整数（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_uint128_be(const void *src, void *value);

/**
 * @brief 型別アンパック関数 - 16ビット浮動小数点数（半精度）（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float16_le(const void *src, float *value);

/**
 * @brief 型別アンパック関数 - 16ビット浮動小数点数（半精度）（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float16_be(const void *src, float *value);

/**
 * @brief 型別アンパック関数 - 32ビット浮動小数点数（単精度）（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float32_le(const void *src, float *value);

/**
 * @brief 型別アンパック関数 - 32ビット浮動小数点数（単精度）（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float32_be(const void *src, float *value);

/**
 * @brief 型別アンパック関数 - 64ビット浮動小数点数（倍精度）（リトルエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float64_le(const void *src, double *value);

/**
 * @brief 型別アンパック関数 - 64ビット浮動小数点数（倍精度）（ビッグエンディアン）
 * @param src 入力元バッファ
 * @param value アンパックした値を格納する変数へのポインタ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_float64_be(const void *src, double *value);

/**
 * @brief 型別パック関数 - 文字列
 * @param dst 出力先バッファ
 * @param value パックする文字列
 * @param size 文字列の最大サイズ
 * @return パック後の次の位置
 */
void *cstruct_pack_string(void *dst, const char *value, size_t size);

/**
 * @brief 型別アンパック関数 - 文字列
 * @param src 入力元バッファ
 * @param value アンパックした文字列を格納する変数へのポインタ
 * @param size 文字列の最大サイズ
 * @return アンパック後の次の位置
 */
const void *cstruct_unpack_string(const void *src, char *value, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* CSTRUCT_H */
