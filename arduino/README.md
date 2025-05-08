# CStruct for Arduino

A lightweight, portable Arduino library for packing and unpacking binary data using Python-style format strings.

## Features

- Supports various data types (8/16/32/64-bit integers, floating point numbers, strings, etc.)
- Supports array processing for all data types
- Supports IEEE754 half precision (16-bit) floating point numbers
- Supports both little-endian and big-endian byte orders
- Simple format string syntax for packing and unpacking
- Individual packing and unpacking functions for each data type

## Installation

1. Download the latest release from the [releases page](https://github.com/sensignal/cstruct/releases)
2. In the Arduino IDE, go to Sketch > Include Library > Add .ZIP Library...
3. Select the downloaded ZIP file
4. The library will be installed and ready to use

## Usage

### Basic Usage

```cpp
#include <CStruct.h>

void setup() {
  Serial.begin(115200);
  
  // Prepare buffer
  uint8_t buffer[32];
  
  // Data to pack
  int16_t value1 = -12345;
  uint32_t value2 = 1234567890;
  float value3 = 3.14159;
  
  // Format string: h=int16, I=uint32, f=float32
  // '<' specifies little-endian
  CStruct::pack(buffer, sizeof(buffer), "<hIf", value1, value2, value3);
  
  // Unpack data
  int16_t unpacked1;
  uint32_t unpacked2;
  float unpacked3;
  
  CStruct::unpack(buffer, sizeof(buffer), "<hIf", &unpacked1, &unpacked2, &unpacked3);
}
```

### Format String

The format string specifies the types and order of data to pack or unpack.

#### Endianness Specifiers

| Symbol | Description |
|--------|-------------|
| <      | Little Endian |
| >      | Big Endian |

Endianness can be switched at any point in the format string and applies to all subsequent data types. The default (initial) endianness is little-endian.

#### Data Type Specifiers

| Symbol | Type | Size (bytes) | Description |
|--------|------|--------------|-------------|
| b      | int8_t | 1 | signed 8-bit integer |
| B      | uint8_t | 1 | unsigned 8-bit integer |
| h      | int16_t | 2 | signed 16-bit integer |
| H      | uint16_t | 2 | unsigned 16-bit integer |
| i      | int32_t | 4 | signed 32-bit integer |
| I      | uint32_t | 4 | unsigned 32-bit integer |
| q      | int64_t | 8 | signed 64-bit integer |
| Q      | uint64_t | 8 | unsigned 64-bit integer |
| e      | float | 2 | IEEE754 half precision (16-bit floating point) |
| f      | float | 4 | IEEE754 float32 (32-bit floating point) |
| d      | double | 8 | IEEE754 float64 (64-bit floating point) |
| s      | char* | 1 | Fixed-length string (N bytes). If N is omitted, defaults to 1. |
| x      | padding | 1 | Skip N bytes. If N is omitted, defaults to 1. |

**Note**: Unlike Python's `struct`, this library allows you to omit the size for `s` and `x`.
In such cases, it defaults to 1 byte. For example, `"s"` is equivalent to `"1s"`, and `"x"` to `"1x"`.

#### Array Notation

All data types can be repeated using a numeric prefix. For example, `3b` means an array of three 8-bit signed integers.

When using arrays, you only need to pass a single argument for the entire array. The library will handle reading or writing multiple elements based on the format string.

```cpp
// Example: Packing and unpacking arrays
int8_t int8_array[3] = {-1, -2, -3};
int8_t unpacked_array[3];

// Pack an array of three 8-bit integers
// Note: Only one argument (the array) is needed
CStruct::pack(buffer, sizeof(buffer), "3b", int8_array);

// Unpack an array of three 8-bit integers
// Note: Only one argument (the array) is needed
CStruct::unpack(buffer, sizeof(buffer), "3b", unpacked_array);
```

#### String Handling

When using the string specifier `s`, the specified number of bytes are copied. When unpacking, a null terminator is added after the copied data. Therefore, the user must provide a buffer that is at least N+1 bytes in size.

```cpp
// Example: Packing and unpacking strings
char str[] = "Hello";
char unpacked_str[6]; // 5 bytes + null terminator

// Pack a 5-byte string
CStruct::pack(buffer, sizeof(buffer), "5s", str);

// Unpack a 5-byte string
CStruct::unpack(buffer, sizeof(buffer), "5s", unpacked_str);
// unpacked_str now contains "Hello" with a null terminator at position 5
```

#### Padding Behavior

When using the padding specifier `x`, the pointer is advanced by N bytes, but no actual write operation is performed. The memory content in the padding area remains unchanged and is skipped.

When using the `pack` and `unpack` functions, no arguments are needed for padding. For example, with the format string `"I4xI"`, the `pack` function only requires two `uint32_t` values as arguments.

## Examples

The library includes the following examples:

### Basic Examples

1. **BasicUsage**: Demonstrates basic operations including data types, strings, arrays, endianness control, and half-precision floating point. Ideal for first-time users of the library.

### Advanced Examples

2. **StringAndArrayHandling**: Focuses on string and array processing. Shows how to handle multiple strings and arrays of various types.

3. **SensorDataPacket**: A practical example of packing and unpacking sensor data as binary packets. Suitable for data transmission via serial communication.

4. **AdvancedUsage**: Demonstrates advanced features including padding usage, complex data structures, and sensor data packet formatting. Note that this example may require more memory than available on Arduino Uno.

## License

This library is released under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).

## Copyright

Copyright (c) 2025 Sensignal Co.,Ltd.
