/* =========================================================================
    CStruct; binary pack/unpack tools for Arduino - String and Array Handling Example
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

/**
 * @file StringAndArrayHandling.ino
 * @brief Example of string and array handling with CStruct library
 * 
 * This sample demonstrates how to use the CStruct library to pack and unpack
 * strings and arrays of various data types.
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
  
  Serial.println("CStruct Arduino Library String and Array Handling Example");
  Serial.println("=======================================================");
  
  // Prepare buffer
  uint8_t buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  
  // String handling example
  stringHandlingExample(buffer);
  
  // Array handling example
  arrayHandlingExample(buffer);
  
  // Combined example (strings and arrays together)
  combinedExample(buffer);
}

void loop() {
  // Nothing to do here
  delay(10000);
}

/**
 * @brief String handling example
 * 
 * Demonstrates how to pack and unpack strings using CStruct
 */
void stringHandlingExample(uint8_t* buffer) {
  Serial.println("\n--- String Handling Example ---");
  
  // Prepare strings for packing
  char shortStr[] = "Hello";
  char longStr[] = "This is a longer string that will be truncated";
  
  // Prepare buffers for unpacking
  char unpackedShortStr[6] = {0};  // 5 bytes + null terminator
  char unpackedLongStr[11] = {0};  // 10 bytes + null terminator
  
  // Format string: 5s = 5-byte string, 10s = 10-byte string
  const char* format = "5s10s";
  
  // Pack the strings
  Serial.println("Packing strings...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format, shortStr, longStr);
  
  if (result == NULL) {
    Serial.println("Packing failed");
    return;
  }
  
  // Display packed binary data
  Serial.println("Packed binary data (hexadecimal):");
  for (int i = 0; i < 15; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
  
  // Unpack the strings
  Serial.println("Unpacking strings...");
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, format, 
                                           unpackedShortStr, unpackedLongStr);
  
  if (unpackResult == NULL) {
    Serial.println("Unpacking failed");
    return;
  }
  
  // Display unpacked strings
  Serial.println("Unpacked strings:");
  Serial.print("Short string: '"); Serial.print(unpackedShortStr); Serial.println("'");
  Serial.print("Long string: '"); Serial.print(unpackedLongStr); Serial.println("'");
  
  // Verify string lengths and null termination
  Serial.println("\nVerifying string properties:");
  Serial.print("Short string length: "); Serial.println(strlen(unpackedShortStr));
  Serial.print("Long string length: "); Serial.println(strlen(unpackedLongStr));
  
  // Check if strings are properly null-terminated
  Serial.print("Short string null-terminated: ");
  Serial.println(unpackedShortStr[5] == '\0' ? "Yes" : "No");
  
  Serial.print("Long string null-terminated: ");
  Serial.println(unpackedLongStr[10] == '\0' ? "Yes" : "No");
}

/**
 * @brief Array handling example
 * 
 * Demonstrates how to pack and unpack arrays using CStruct
 */
void arrayHandlingExample(uint8_t* buffer) {
  Serial.println("\n--- Array Handling Example ---");
  
  // Prepare arrays for packing
  int8_t int8Array[3] = {-1, -2, -3};
  uint16_t uint16Array[4] = {1000, 2000, 3000, 4000};
  float floatArray[2] = {3.14159, 2.71828};
  
  // Prepare arrays for unpacking
  int8_t unpackedInt8Array[3] = {0};
  uint16_t unpackedUint16Array[4] = {0};
  float unpackedFloatArray[2] = {0.0};
  
  // Format string: 3b = array of 3 int8_t, 4H = array of 4 uint16_t, 2f = array of 2 float
  const char* format = "3b4H2f";
  
  // Pack the arrays
  Serial.println("Packing arrays...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format, 
                             int8Array, uint16Array, floatArray);
  
  if (result == NULL) {
    Serial.println("Packing failed");
    return;
  }
  
  // Display packed binary data
  Serial.println("Packed binary data (hexadecimal):");
  for (int i = 0; i < 19; i++) {  // 3 + 8 + 8 = 19 bytes
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
  
  // Unpack the arrays
  Serial.println("Unpacking arrays...");
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, format, 
                                           unpackedInt8Array, 
                                           unpackedUint16Array, 
                                           unpackedFloatArray);
  
  if (unpackResult == NULL) {
    Serial.println("Unpacking failed");
    return;
  }
  
  // Display unpacked arrays
  Serial.println("Unpacked arrays:");
  
  Serial.print("int8 array: ");
  for (int i = 0; i < 3; i++) {
    Serial.print(unpackedInt8Array[i]);
    if (i < 2) Serial.print(", ");
  }
  Serial.println();
  
  Serial.print("uint16 array: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(unpackedUint16Array[i]);
    if (i < 3) Serial.print(", ");
  }
  Serial.println();
  
  Serial.print("float array: ");
  for (int i = 0; i < 2; i++) {
    Serial.print(unpackedFloatArray[i], 5);
    if (i < 1) Serial.print(", ");
  }
  Serial.println();
  
  // Verify array values
  Serial.println("\nVerifying array values:");
  bool allMatch = true;
  
  for (int i = 0; i < 3; i++) {
    if (unpackedInt8Array[i] != int8Array[i]) {
      Serial.print("int8 array mismatch at index "); Serial.println(i);
      allMatch = false;
    }
  }
  
  for (int i = 0; i < 4; i++) {
    if (unpackedUint16Array[i] != uint16Array[i]) {
      Serial.print("uint16 array mismatch at index "); Serial.println(i);
      allMatch = false;
    }
  }
  
  for (int i = 0; i < 2; i++) {
    if (abs(unpackedFloatArray[i] - floatArray[i]) > 0.00001) {
      Serial.print("float array mismatch at index "); Serial.println(i);
      allMatch = false;
    }
  }
  
  if (allMatch) {
    Serial.println("All array values match! Data integrity verified.");
  }
}

/**
 * @brief Combined example with strings and arrays
 * 
 * Demonstrates how to pack and unpack a combination of strings and arrays
 */
void combinedExample(uint8_t* buffer) {
  Serial.println("\n--- Combined Example (Strings and Arrays) ---");
  
  // Prepare data for packing
  char deviceName[] = "Arduino";
  uint8_t sensorIds[3] = {1, 2, 3};
  float readings[3] = {25.4, 1013.25, 65.8};  // Temperature, Pressure, Humidity
  
  // Prepare buffers for unpacking
  char unpackedDeviceName[8] = {0};  // 7 bytes + null terminator
  uint8_t unpackedSensorIds[3] = {0};
  float unpackedReadings[3] = {0.0};
  
  // Format string: 7s = 7-byte string, 3B = array of 3 uint8_t, 3f = array of 3 float
  const char* format = "7s3B3f";
  
  // Pack the data
  Serial.println("Packing combined data...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format, 
                             deviceName, sensorIds, readings);
  
  if (result == NULL) {
    Serial.println("Packing failed");
    return;
  }
  
  // Display packed binary data
  Serial.println("Packed binary data (hexadecimal):");
  for (int i = 0; i < 22; i++) {  // 7 + 3 + 12 = 22 bytes
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
  
  // Unpack the data
  Serial.println("Unpacking combined data...");
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, format, 
                                           unpackedDeviceName, 
                                           unpackedSensorIds, 
                                           unpackedReadings);
  
  if (unpackResult == NULL) {
    Serial.println("Unpacking failed");
    return;
  }
  
  // Display unpacked data
  Serial.println("Unpacked data:");
  Serial.print("Device name: "); Serial.println(unpackedDeviceName);
  
  Serial.print("Sensor IDs: ");
  for (int i = 0; i < 3; i++) {
    Serial.print(unpackedSensorIds[i]);
    if (i < 2) Serial.print(", ");
  }
  Serial.println();
  
  Serial.println("Sensor readings:");
  Serial.print("Temperature: "); Serial.print(unpackedReadings[0]); Serial.println(" °C");
  Serial.print("Pressure: "); Serial.print(unpackedReadings[1]); Serial.println(" hPa");
  Serial.print("Humidity: "); Serial.print(unpackedReadings[2]); Serial.println(" %");
}
