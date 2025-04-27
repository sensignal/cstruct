/* =========================================================================
    CStruct; binary pack/unpack tools for Arduino.
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

/**
 * @file CStruct.cpp
 * @brief Implementation of Arduino library for packing and unpacking binary data
 *
 * This file provides the implementation of the CStruct class.
 * Internally, it calls the functions from the C library.
 */

#include "CStruct.h"
#include <stdarg.h>

// Include the original CStruct library header
extern "C" {
    #include "../../c/cstruct.h"
}

// Implementation of pack function
void* CStruct::pack(void* dst, size_t dstlen, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    void* result = cstruct_pack_v(dst, dstlen, fmt, args);
    va_end(args);
    return result;
}

// Implementation of unpack function
const void* CStruct::unpack(const void* src, size_t srclen, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const void* result = cstruct_unpack_v(src, srclen, fmt, args);
    va_end(args);
    return result;
}

// Implementation of getPtr function
const void* CStruct::getPtr(const void* src, size_t srclen, const char* fmt, size_t index) {
    return cstruct_get_ptr(src, srclen, fmt, index);
}

// Implementation of type-specific pack functions
void* CStruct::packPadding(void* dst, size_t size) {
    return cstruct_pack_padding(dst, size);
}

void* CStruct::packInt8(void* dst, int8_t value) {
    return cstruct_pack_int8(dst, value);
}

void* CStruct::packUint8(void* dst, uint8_t value) {
    return cstruct_pack_uint8(dst, value);
}

void* CStruct::packInt16LE(void* dst, int16_t value) {
    return cstruct_pack_int16_le(dst, value);
}

void* CStruct::packInt16BE(void* dst, int16_t value) {
    return cstruct_pack_int16_be(dst, value);
}

void* CStruct::packUint16LE(void* dst, uint16_t value) {
    return cstruct_pack_uint16_le(dst, value);
}

void* CStruct::packUint16BE(void* dst, uint16_t value) {
    return cstruct_pack_uint16_be(dst, value);
}

void* CStruct::packInt32LE(void* dst, int32_t value) {
    return cstruct_pack_int32_le(dst, value);
}

void* CStruct::packInt32BE(void* dst, int32_t value) {
    return cstruct_pack_int32_be(dst, value);
}

void* CStruct::packUint32LE(void* dst, uint32_t value) {
    return cstruct_pack_uint32_le(dst, value);
}

void* CStruct::packUint32BE(void* dst, uint32_t value) {
    return cstruct_pack_uint32_be(dst, value);
}

void* CStruct::packInt64LE(void* dst, int64_t value) {
    return cstruct_pack_int64_le(dst, value);
}

void* CStruct::packInt64BE(void* dst, int64_t value) {
    return cstruct_pack_int64_be(dst, value);
}

void* CStruct::packUint64LE(void* dst, uint64_t value) {
    return cstruct_pack_uint64_le(dst, value);
}

void* CStruct::packUint64BE(void* dst, uint64_t value) {
    return cstruct_pack_uint64_be(dst, value);
}

void* CStruct::packFloat16LE(void* dst, float value) {
    return cstruct_pack_float16_le(dst, value);
}

void* CStruct::packFloat16BE(void* dst, float value) {
    return cstruct_pack_float16_be(dst, value);
}

void* CStruct::packFloat32LE(void* dst, float value) {
    return cstruct_pack_float32_le(dst, value);
}

void* CStruct::packFloat32BE(void* dst, float value) {
    return cstruct_pack_float32_be(dst, value);
}

void* CStruct::packFloat64LE(void* dst, double value) {
    return cstruct_pack_float64_le(dst, value);
}

void* CStruct::packFloat64BE(void* dst, double value) {
    return cstruct_pack_float64_be(dst, value);
}

// Implementation of type-specific unpack functions
const void* CStruct::unpackInt8(const void* src, int8_t* value) {
    return cstruct_unpack_int8(src, value);
}

const void* CStruct::unpackUint8(const void* src, uint8_t* value) {
    return cstruct_unpack_uint8(src, value);
}

const void* CStruct::unpackInt16LE(const void* src, int16_t* value) {
    return cstruct_unpack_int16_le(src, value);
}

const void* CStruct::unpackInt16BE(const void* src, int16_t* value) {
    return cstruct_unpack_int16_be(src, value);
}

const void* CStruct::unpackUint16LE(const void* src, uint16_t* value) {
    return cstruct_unpack_uint16_le(src, value);
}

const void* CStruct::unpackUint16BE(const void* src, uint16_t* value) {
    return cstruct_unpack_uint16_be(src, value);
}

const void* CStruct::unpackInt32LE(const void* src, int32_t* value) {
    return cstruct_unpack_int32_le(src, value);
}

const void* CStruct::unpackInt32BE(const void* src, int32_t* value) {
    return cstruct_unpack_int32_be(src, value);
}

const void* CStruct::unpackUint32LE(const void* src, uint32_t* value) {
    return cstruct_unpack_uint32_le(src, value);
}

const void* CStruct::unpackUint32BE(const void* src, uint32_t* value) {
    return cstruct_unpack_uint32_be(src, value);
}

const void* CStruct::unpackInt64LE(const void* src, int64_t* value) {
    return cstruct_unpack_int64_le(src, value);
}

const void* CStruct::unpackInt64BE(const void* src, int64_t* value) {
    return cstruct_unpack_int64_be(src, value);
}

const void* CStruct::unpackUint64LE(const void* src, uint64_t* value) {
    return cstruct_unpack_uint64_le(src, value);
}

const void* CStruct::unpackUint64BE(const void* src, uint64_t* value) {
    return cstruct_unpack_uint64_be(src, value);
}

const void* CStruct::unpackFloat16LE(const void* src, float* value) {
    return cstruct_unpack_float16_le(src, value);
}

const void* CStruct::unpackFloat16BE(const void* src, float* value) {
    return cstruct_unpack_float16_be(src, value);
}

const void* CStruct::unpackFloat32LE(const void* src, float* value) {
    return cstruct_unpack_float32_le(src, value);
}

const void* CStruct::unpackFloat32BE(const void* src, float* value) {
    return cstruct_unpack_float32_be(src, value);
}

const void* CStruct::unpackFloat64LE(const void* src, double* value) {
    return cstruct_unpack_float64_le(src, value);
}

const void* CStruct::unpackFloat64BE(const void* src, double* value) {
    return cstruct_unpack_float64_be(src, value);
}
