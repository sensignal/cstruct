# CStruct for Arduino

An Arduino library for packing and unpacking binary data with a simple format string syntax.

## Features

- Supports various data types (8/16/32/64-bit integers, floating point numbers, etc.)
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

#### Special Fields

| Symbol | Type | Size | Description |
|--------|------|------|-------------|
| xN     | padding | N bytes | N bytes of padding. N is optional (defaults to 1 if omitted) and must be greater than 0. |

When using the padding specifier `x`, the pointer is advanced by N bytes, but no actual write operation is performed. The memory content in the padding area remains unchanged and is skipped.

## Examples

The library includes the following examples:

1. **BasicUsage**: Demonstrates basic packing and unpacking operations
2. **SensorDataPacket**: Demonstrates packing and unpacking sensor data as a binary packet

## License

This library is released under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).

## Copyright

Copyright (c) 2025 Sensignal Co.,Ltd.
