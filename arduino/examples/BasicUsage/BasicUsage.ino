/* =========================================================================
    CStruct; binary pack/unpack tools for Arduino - Basic Usage Example
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

/**
 * @file BasicUsage.ino
 * @brief Basic usage example of the CStruct library
 * 
 * This sample demonstrates how to use the CStruct library to pack and unpack:
 * - Basic data types (integers, floats)
 * - Strings
 * - Arrays
 * - Endianness control
 * - Half-precision floating point
 */

#include <CStruct.h>

// Define buffer size
#define BUFFER_SIZE 128

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for USB connection only)
  }
  
  Serial.println("CStruct Arduino Library Basic Usage Example");
  Serial.println("===========================================");
  
  // Prepare buffer
  uint8_t buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  
  // Example 1: Basic data types
  basicDataTypesExample(buffer);
  
  // Example 2: String handling
  stringHandlingExample(buffer);
  
  // Example 3: Array handling
  arrayHandlingExample(buffer);
  
  // Example 4: Endianness control
  endiannessExample(buffer);
  
  // Example 5: Half-precision floating point
  halfPrecisionExample(buffer);
}

void loop() {
  // Nothing to do here
  delay(10000);
}

/**
 * @brief Example 4: Endianness control
 * 
 * Demonstrates how to control byte order (endianness) in packed data
 */
void endiannessExample(uint8_t* buffer) {
  Serial.println("\n--- Example 4: Endianness Control ---");
  
  // テスト値 (0x12345678 16進数)
  uint32_t testValue = 0x12345678;
  uint32_t unpackedLittleEndian, unpackedBigEndian;
  
  Serial.print("Original: 0x");
  Serial.println(testValue, HEX);
  
  // リトルエンディアンでパック
  memset(buffer, 0, BUFFER_SIZE);
  void* result = CStruct::pack(buffer, BUFFER_SIZE, "<I", testValue);
  
  if (result == NULL) {
    Serial.println("Pack failed");
    return;
  }
  
  // パックしたデータを表示
  Serial.print("Little-endian: ");
  for (int i = 0; i < 4; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); // 78 56 34 12 になるはず
  
  // リトルエンディアンでアンパック
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, "<I", &unpackedLittleEndian);
  
  if (unpackResult == NULL) {
    Serial.println("Unpack failed");
    return;
  }
  
  // ビッグエンディアンでパック
  memset(buffer, 0, BUFFER_SIZE);
  result = CStruct::pack(buffer, BUFFER_SIZE, ">I", testValue);
  
  if (result == NULL) {
    Serial.println("Pack failed");
    return;
  }
  
  // パックしたデータを表示
  Serial.print("Big-endian: ");
  for (int i = 0; i < 4; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); // 12 34 56 78 になるはず
  
  // ビッグエンディアンでアンパック
  unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, ">I", &unpackedBigEndian);
  
  if (unpackResult == NULL) {
    Serial.println("Unpack failed");
    return;
  }
  
  // アンパックした値を検証
  Serial.print("Unpacked LE: 0x");
  Serial.println(unpackedLittleEndian, HEX);
  
  Serial.print("Unpacked BE: 0x");
  Serial.println(unpackedBigEndian, HEX);
  
  // 混合エンディアンの例
  uint16_t value1 = 0x1234;
  uint16_t value2 = 0x5678;
  uint16_t unpackedValue1, unpackedValue2;
  
  // 混合エンディアンのフォーマット文字列
  memset(buffer, 0, BUFFER_SIZE);
  result = CStruct::pack(buffer, BUFFER_SIZE, "<H>H", value1, value2);
  
  if (result == NULL) {
    Serial.println("Pack failed");
    return;
  }
  
  // アンパック
  unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, "<H>H", &unpackedValue1, &unpackedValue2);
  
  if (unpackResult == NULL) {
    Serial.println("Unpack failed");
    return;
  }
  
  Serial.print("Mixed endian: ");
  Serial.print(unpackedValue1, HEX);
  Serial.print(", ");
  Serial.println(unpackedValue2, HEX);
}

/**
 * @brief Example 5: Half-precision floating point
 * 
 * Demonstrates how to use half-precision (16-bit) floating point values
 */
void halfPrecisionExample(uint8_t* buffer) {
  Serial.println("\n--- Example 5: Half-Precision Float ---");
  
  // ハーフプレシジョンで正確に表現できるテスト値
  float testValues[] = {0.0, 1.0, -2.0, 10.5};
  float unpackedValues[4];
  
  // フォーマット文字列: 4e = 4つのハーフプレシジョン浮動小数点
  const char* format = "4e";
  
  // ハーフプレシジョン値をパック
  Serial.println("Packing half-precision...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format, 
                            testValues[0], testValues[1], 
                            testValues[2], testValues[3]);
  
  if (result == NULL) {
    Serial.println("Pack failed");
    return;
  }
  
  // パックしたバイナリデータを表示 (8バイト)
  Serial.print("Packed data: ");
  for (int i = 0; i < 8; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // ハーフプレシジョン値をアンパック
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, format,
                                          &unpackedValues[0], &unpackedValues[1], 
                                          &unpackedValues[2], &unpackedValues[3]);
  
  if (unpackResult == NULL) {
    Serial.println("Unpack failed");
    return;
  }
  
  // 元の値とアンパックした値を表示
  Serial.println("Original vs Unpacked:");
  for (int i = 0; i < 4; i++) {
    Serial.print(testValues[i]);
    Serial.print(" -> ");
    Serial.println(unpackedValues[i], 4);
  }
}

/**
 * @brief Example 1: Basic data types
 * 
 * Demonstrates packing and unpacking basic data types
 */
void basicDataTypesExample(uint8_t* buffer) {
  Serial.println("\n--- Example 1: Basic Data Types ---");
  
  // Prepare data to pack
  int8_t int8Value = -42;
  uint8_t uint8Value = 200;
  int16_t int16Value = -12345;
  uint16_t uint16Value = 54321;
  int32_t int32Value = -1234567890;
  uint32_t uint32Value = 3456789012;
  float floatValue = 3.14159;
  
  Serial.println("Data to pack:");
  Serial.print("int8: "); Serial.println(int8Value);
  Serial.print("uint8: "); Serial.println(uint8Value);
  Serial.print("int16: "); Serial.println(int16Value);
  Serial.print("uint16: "); Serial.println(uint16Value);
  Serial.print("int32: "); Serial.println(int32Value);
  Serial.print("uint32: "); Serial.println(uint32Value);
  Serial.print("float: "); Serial.println(floatValue, 5);
  
  // Format string: b=int8, B=uint8, h=int16, H=uint16, i=int32, I=uint32, f=float32
  const char* format = "bBhHiIf";
  
  // Pack data
  Serial.println("\nPacking data...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format,
                             int8Value, uint8Value, int16Value, uint16Value,
                             int32Value, uint32Value, floatValue);
  
  if (result == NULL) {
    Serial.println("Packing failed");
    return;
  }
  
  // Display packed binary data (first 23 bytes)
  Serial.println("Packed binary data (hexadecimal):");
  for (int i = 0; i < 23; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
  
  // Prepare variables for unpacking
  int8_t unpackedInt8;
  uint8_t unpackedUint8;
  int16_t unpackedInt16;
  uint16_t unpackedUint16;
  int32_t unpackedInt32;
  uint32_t unpackedUint32;
  float unpackedFloat;
  
  // Unpack data
  Serial.println("Unpacking data...");
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, format,
                                           &unpackedInt8, &unpackedUint8,
                                           &unpackedInt16, &unpackedUint16,
                                           &unpackedInt32, &unpackedUint32,
                                           &unpackedFloat);
  
  if (unpackResult == NULL) {
    Serial.println("Unpacking failed");
    return;
  }
  
  // Display unpacked data
  Serial.println("Unpacked data:");
  Serial.print("int8: "); Serial.println(unpackedInt8);
  Serial.print("uint8: "); Serial.println(unpackedUint8);
  Serial.print("int16: "); Serial.println(unpackedInt16);
  Serial.print("uint16: "); Serial.println(unpackedUint16);
  Serial.print("int32: "); Serial.println(unpackedInt32);
  Serial.print("uint32: "); Serial.println(unpackedUint32);
  Serial.print("float: "); Serial.println(unpackedFloat, 5);
}

/**
 * @brief Example 2: String handling
 * 
 * Demonstrates packing and unpacking strings
 */
void stringHandlingExample(uint8_t* buffer) {
  Serial.println("\n--- Example 2: String Handling ---");
  
  // Prepare strings for packing
  char deviceName[] = "Arduino";
  char message[] = "Hello World";
  
  // Prepare buffers for unpacking
  char unpackedDeviceName[8] = {0};  // 7 bytes + null terminator
  char unpackedMessage[12] = {0};    // 11 bytes + null terminator
  
  // Format string: 7s = 7-byte string, 11s = 11-byte string
  const char* format = "7s11s";
  
  // Pack the strings
  Serial.println("Packing strings...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format, deviceName, message);
  
  if (result == NULL) {
    Serial.println("Packing failed");
    return;
  }
  
  // Display packed binary data (first 18 bytes)
  Serial.println("Packed binary data (hexadecimal):");
  for (int i = 0; i < 18; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
  
  // Unpack the strings
  Serial.println("Unpacking strings...");
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, format, 
                                           unpackedDeviceName, unpackedMessage);
  
  if (unpackResult == NULL) {
    Serial.println("Unpacking failed");
    return;
  }
  
  // Display unpacked strings
  Serial.println("Unpacked strings:");
  Serial.print("Device name: '"); Serial.print(unpackedDeviceName); Serial.println("'");
  Serial.print("Message: '"); Serial.print(unpackedMessage); Serial.println("'");
  
  // Verify null termination
  Serial.println("\nVerifying null termination:");
  Serial.print("Device name is null-terminated: ");
  Serial.println(unpackedDeviceName[7] == '\0' ? "Yes" : "No");
  
  Serial.print("Message is null-terminated: ");
  Serial.println(unpackedMessage[11] == '\0' ? "Yes" : "No");
}

/**
 * @brief Example 3: Array handling
 * 
 * Demonstrates packing and unpacking arrays
 */
void arrayHandlingExample(uint8_t* buffer) {
  Serial.println("\n--- Example 3: Array Handling ---");
  
  // Prepare arrays for packing
  int8_t temperatures[3] = {21, 22, 23};  // Temperature readings in °C
  uint16_t adcValues[4] = {512, 768, 1024, 1536};  // ADC readings
  
  // Prepare arrays for unpacking
  int8_t unpackedTemperatures[3] = {0};
  uint16_t unpackedAdcValues[4] = {0};
  
  // Format string: 3b = array of 3 int8_t, 4H = array of 4 uint16_t
  const char* format = "3b4H";
  
  // Pack the arrays
  Serial.println("Packing arrays...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format, 
                             temperatures, adcValues);
  
  if (result == NULL) {
    Serial.println("Packing failed");
    return;
  }
  
  // Display packed binary data (first 11 bytes)
  Serial.println("Packed binary data (hexadecimal):");
  for (int i = 0; i < 11; i++) {  // 3 + 8 = 11 bytes
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
  
  // Unpack the arrays
  Serial.println("Unpacking arrays...");
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, format, 
                                           unpackedTemperatures, 
                                           unpackedAdcValues);
  
  if (unpackResult == NULL) {
    Serial.println("Unpacking failed");
    return;
  }
  
  // Display unpacked arrays
  Serial.println("Unpacked arrays:");
  
  Serial.print("Temperatures: ");
  for (int i = 0; i < 3; i++) {
    Serial.print(unpackedTemperatures[i]);
    Serial.print("°C");
    if (i < 2) Serial.print(", ");
  }
  Serial.println();
  
  Serial.print("ADC Values: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(unpackedAdcValues[i]);
    if (i < 3) Serial.print(", ");
  }
  Serial.println();
}
