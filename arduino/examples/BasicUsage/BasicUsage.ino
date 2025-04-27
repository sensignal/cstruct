/* =========================================================================
    CStruct; binary pack/unpack tools for Arduino - Basic Usage Example
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

/**
 * @file BasicUsage.ino
 * @brief Basic usage example of the CStruct library
 * 
 * This sample demonstrates how to use the CStruct library to pack various types
 * of data into binary format and then unpack them.
 */

#include <CStruct.h>

// Define buffer size
#define BUFFER_SIZE 64

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
  
  // Pack data
  Serial.println("\nPacking data...");
  
  // Format string: b=int8, B=uint8, h=int16, H=uint16, i=int32, I=uint32, f=float32
  // '<' specifies little-endian, '>' specifies big-endian
  const char* format = "<bBhHiIf";
  
  // Packing process
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format,
                              int8Value, uint8Value, int16Value, uint16Value,
                              int32Value, uint32Value, floatValue);
  
  if (result == NULL) {
    Serial.println("Packing failed");
    return;
  }
  
  // Display packed binary data
  Serial.println("Packed binary data (hexadecimal):");
  for (int i = 0; i < 23; i++) {  // 8bit + 8bit + 16bit + 16bit + 32bit + 32bit + 32bit = 184bit = 23 bytes
    if (buffer[i] < 0x10) Serial.print("0");  // Add leading zero for single digit
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();  // Line break every 8 bytes
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
  
  // Verify if unpacked data matches original data
  Serial.println("\nVerifying data integrity:");
  bool allMatch = true;
  
  if (unpackedInt8 != int8Value) {
    Serial.println("int8 values do not match!");
    allMatch = false;
  }
  
  if (unpackedUint8 != uint8Value) {
    Serial.println("uint8 values do not match!");
    allMatch = false;
  }
  
  if (unpackedInt16 != int16Value) {
    Serial.println("int16 values do not match!");
    allMatch = false;
  }
  
  if (unpackedUint16 != uint16Value) {
    Serial.println("uint16 values do not match!");
    allMatch = false;
  }
  
  if (unpackedInt32 != int32Value) {
    Serial.println("int32 values do not match!");
    allMatch = false;
  }
  
  if (unpackedUint32 != uint32Value) {
    Serial.println("uint32 values do not match!");
    allMatch = false;
  }
  
  // For floating point, use a small epsilon for comparison
  if (abs(unpackedFloat - floatValue) > 0.00001) {
    Serial.println("float values do not match!");
    allMatch = false;
  }
  
  if (allMatch) {
    Serial.println("All values match! Data integrity verified.");
  }
  
  // Half precision floating point (16-bit) test
  Serial.println("\nHalf precision floating point (16-bit) test");
  
  float float16TestValue = 12.375;  // Half precision value that can be accurately represented
  uint16_t packedFloat16;
  
  // Pack half precision floating point
  CStruct::packFloat16LE(&packedFloat16, float16TestValue);
  
  Serial.print("Original value: "); Serial.println(float16TestValue, 5);
  Serial.print("Packed 16-bit value (hexadecimal): 0x");
  if ((packedFloat16 >> 8) < 0x10) Serial.print("0");
  Serial.print((packedFloat16 >> 8) & 0xFF, HEX);
  if ((packedFloat16 & 0xFF) < 0x10) Serial.print("0");
  Serial.println(packedFloat16 & 0xFF, HEX);
  
  // Unpack half precision floating point
  float unpackedFloat16;
  CStruct::unpackFloat16LE(&packedFloat16, &unpackedFloat16);
  
  Serial.print("Unpacked value: "); Serial.println(unpackedFloat16, 5);
}

void loop() {
  // Nothing to do here
}
