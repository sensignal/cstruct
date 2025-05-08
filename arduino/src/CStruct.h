/* =========================================================================
    CStruct; binary pack/unpack tools for Arduino.
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

/**
 * @file CStruct.h
 * @brief Header file for Arduino library for packing and unpacking binary data
 *
 * This library provides functions for converting structured data to binary format (packing)
 * and converting binary data back to structured data (unpacking).
 *
 * # Endianness Specifiers
 * Symbol  Description
 * <       Little Endian
 * >       Big Endian
 * Endianness can be switched at any point in the format string
 * Applies to all subsequent data types after the specifier
 * Default (initial) is little-endian
 *
 * # Data Type Specifiers
 * Symbol  Type        Size (bytes)    Description
 * b       int8_t      1               signed 8-bit integer
 * B       uint8_t     1               unsigned 8-bit integer
 * h       int16_t     2               signed 16-bit integer
 * H       uint16_t    2               unsigned 16-bit integer
 * i       int32_t     4               signed 32-bit integer
 * I       uint32_t    4               unsigned 32-bit integer
 * q       int64_t     8               signed 64-bit integer
 * Q       uint64_t    8               unsigned 64-bit integer
 * e       float       2               IEEE754 half precision (16-bit floating point)
 * f       float       4               IEEE754 float32 (32-bit floating point)
 * d       double      8               IEEE754 float64 (64-bit floating point)
 *
 * # Special Fields
 * Symbol  Type        Size            Description
 * xN      padding     N bytes         N bytes of zero padding
 * xN: values are always filled with 0x00
 * N is specified as a decimal number (e.g., x3)
 */

#ifndef CSTRUCT_ARDUINO_H
#define CSTRUCT_ARDUINO_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief CStruct class - Class for packing and unpacking binary data
 */
class CStruct {
public:
    /**
     * @brief Pack data into binary format
     * 
     * Converts variable arguments to binary format according to the specified format string
     * and stores the result in the provided buffer.
     *
     * @param dst Destination buffer
     * @param dstlen Size of destination buffer
     * @param fmt Format string
     * @param ... Values corresponding to the format string
     * @return Pointer to the next position after packing, NULL on error
     */
    static void* pack(void* dst, size_t dstlen, const char* fmt, ...);

    /**
     * @brief Unpack data from binary format
     * 
     * Converts binary data to structured data according to the specified format string
     * and stores the results in the variables provided as pointers.
     *
     * @param src Source buffer
     * @param srclen Size of source buffer
     * @param fmt Format string
     * @param ... Pointers to variables to store unpacked values
     * @return Pointer to the next position after unpacking, NULL on error
     */
    static const void* unpack(const void* src, size_t srclen, const char* fmt, ...);

    /**
     * @brief Get pointer to a field at the specified index
     * 
     * @param src Source buffer
     * @param srclen Size of source buffer
     * @param fmt Format string
     * @param index Index of the field to retrieve (0-based)
     * @return Pointer to the field, NULL on error
     */
    static const void* getPtr(const void* src, size_t srclen, const char* fmt, size_t index);

    /**
     * @brief Type-specific pack function - Padding
     * @param dst Destination buffer
     * @param size Padding size
     * @return Pointer to the next position after packing
     */
    static void* packPadding(void* dst, size_t size);

    /**
     * @brief Type-specific pack function - 8-bit signed integer
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packInt8(void* dst, int8_t value);

    /**
     * @brief Type-specific pack function - 8-bit unsigned integer
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packUint8(void* dst, uint8_t value);

    /**
     * @brief Type-specific pack function - 16-bit signed integer (little-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packInt16LE(void* dst, int16_t value);

    /**
     * @brief Type-specific pack function - 16-bit signed integer (big-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packInt16BE(void* dst, int16_t value);

    /**
     * @brief Type-specific pack function - 16-bit unsigned integer (little-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packUint16LE(void* dst, uint16_t value);

    /**
     * @brief Type-specific pack function - 16-bit unsigned integer (big-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packUint16BE(void* dst, uint16_t value);

    /**
     * @brief Type-specific pack function - 32-bit signed integer (little-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packInt32LE(void* dst, int32_t value);

    /**
     * @brief Type-specific pack function - 32-bit signed integer (big-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packInt32BE(void* dst, int32_t value);

    /**
     * @brief Type-specific pack function - 32-bit unsigned integer (little-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packUint32LE(void* dst, uint32_t value);

    /**
     * @brief Type-specific pack function - 32-bit unsigned integer (big-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packUint32BE(void* dst, uint32_t value);

    /**
     * @brief Type-specific pack function - 64-bit signed integer (little-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packInt64LE(void* dst, int64_t value);

    /**
     * @brief Type-specific pack function - 64-bit signed integer (big-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packInt64BE(void* dst, int64_t value);

    /**
     * @brief Type-specific pack function - 64-bit unsigned integer (little-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packUint64LE(void* dst, uint64_t value);

    /**
     * @brief Type-specific pack function - 64-bit unsigned integer (big-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packUint64BE(void* dst, uint64_t value);

    /**
     * @brief Type-specific pack function - 16-bit floating point (little-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packFloat16LE(void* dst, float value);

    /**
     * @brief Type-specific pack function - 16-bit floating point (big-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packFloat16BE(void* dst, float value);

    /**
     * @brief Type-specific pack function - 32-bit floating point (little-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packFloat32LE(void* dst, float value);

    /**
     * @brief Type-specific pack function - 32-bit floating point (big-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packFloat32BE(void* dst, float value);

    /**
     * @brief Type-specific pack function - 64-bit floating point (little-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packFloat64LE(void* dst, double value);

    /**
     * @brief Type-specific pack function - 64-bit floating point (big-endian)
     * @param dst Destination buffer
     * @param value Value to pack
     * @return Pointer to the next position after packing
     */
    static void* packFloat64BE(void* dst, double value);

    /**
     * @brief Type-specific unpack function - 8-bit signed integer
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackInt8(const void* src, int8_t* value);

    /**
     * @brief Type-specific unpack function - 8-bit unsigned integer
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackUint8(const void* src, uint8_t* value);

    /**
     * @brief Type-specific unpack function - 16-bit signed integer (little-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackInt16LE(const void* src, int16_t* value);

    /**
     * @brief Type-specific unpack function - 16-bit signed integer (big-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackInt16BE(const void* src, int16_t* value);

    /**
     * @brief Type-specific unpack function - 16-bit unsigned integer (little-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackUint16LE(const void* src, uint16_t* value);

    /**
     * @brief Type-specific unpack function - 16-bit unsigned integer (big-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackUint16BE(const void* src, uint16_t* value);

    /**
     * @brief Type-specific unpack function - 32-bit signed integer (little-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackInt32LE(const void* src, int32_t* value);

    /**
     * @brief Type-specific unpack function - 32-bit signed integer (big-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackInt32BE(const void* src, int32_t* value);

    /**
     * @brief Type-specific unpack function - 32-bit unsigned integer (little-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackUint32LE(const void* src, uint32_t* value);

    /**
     * @brief Type-specific unpack function - 32-bit unsigned integer (big-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackUint32BE(const void* src, uint32_t* value);

    /**
     * @brief Type-specific unpack function - 64-bit signed integer (little-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackInt64LE(const void* src, int64_t* value);

    /**
     * @brief Type-specific unpack function - 64-bit signed integer (big-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackInt64BE(const void* src, int64_t* value);

    /**
     * @brief Type-specific unpack function - 64-bit unsigned integer (little-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackUint64LE(const void* src, uint64_t* value);

    /**
     * @brief Type-specific unpack function - 64-bit unsigned integer (big-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackUint64BE(const void* src, uint64_t* value);

    /**
     * @brief Type-specific unpack function - 16-bit floating point (little-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackFloat16LE(const void* src, float* value);

    /**
     * @brief Type-specific unpack function - 16-bit floating point (big-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackFloat16BE(const void* src, float* value);

    /**
     * @brief Type-specific unpack function - 32-bit floating point (little-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackFloat32LE(const void* src, float* value);

    /**
     * @brief Type-specific unpack function - 32-bit floating point (big-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackFloat32BE(const void* src, float* value);

    /**
     * @brief Type-specific unpack function - 64-bit floating point (little-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackFloat64LE(const void* src, double* value);

    /**
     * @brief Type-specific unpack function - 64-bit floating point (big-endian)
     * @param src Source buffer
     * @param value Pointer to store unpacked value
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackFloat64BE(const void* src, double* value);

    /**
     * @brief Type-specific pack function - String
     * @param dst Destination buffer
     * @param value String to pack
     * @param size Size of the string to pack
     * @return Pointer to the next position after packing
     */
    static void* packString(void* dst, const char* value, size_t size);

    /**
     * @brief Type-specific unpack function - String
     * @param src Source buffer
     * @param value Pointer to store unpacked string
     * @param size Size of the string to unpack
     * @return Pointer to the next position after unpacking
     */
    static const void* unpackString(const void* src, char* value, size_t size);

private:
    // Internal implementation functions and variables are defined here
};

#endif // CSTRUCT_ARDUINO_H
