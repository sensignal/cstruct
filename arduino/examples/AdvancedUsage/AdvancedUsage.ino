/* =========================================================================
    CStruct; binary pack/unpack tools for Arduino - Advanced Usage Example
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

/**
 * @file AdvancedUsage.ino
 * @brief Advanced usage examples of the CStruct library
 * 
 * This sample demonstrates advanced features of the CStruct library:
 * - Padding usage
 * - Complex data structures with mixed types
 * - Sensor data packet formatting
 */

#include <CStruct.h>

// Define buffer size (reduced for Arduino Uno compatibility)
#define BUFFER_SIZE 64

// Packet format definition for sensor data
#define PACKET_HEADER 0xAA55  // Packet header
#define PACKET_FORMAT "HB3b4H2f7s"  // Format: Header(H), PacketID(B), Temperatures(3b), 
                                    // ADC values(4H), Sensor readings(2f), Device name(7s)

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for USB connection only)
  }
  
  Serial.println("CStruct Advanced Usage");
  
  // Prepare buffer
  uint8_t buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  
  // Example 1: Padding usage
  paddingExample(buffer);
  
  // Example 2: Complex data structures
  complexDataStructureExample(buffer);
  
  // Example 3: Sensor data packet
  sensorDataPacketExample(buffer);
}

void loop() {
  // Nothing to do here
  delay(10000);
}





/**
 * @brief Example 1: Padding usage
 * 
 * Demonstrates how to use padding in format strings
 */
void paddingExample(uint8_t* buffer) {
  Serial.println("\n--- Example 3: Padding Usage ---");
  
  // Fill buffer with a pattern to visualize padding
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = 0xAA;  // Pattern: 10101010
  }
  
  // Prepare test values
  uint16_t value1 = 0x1234;
  uint16_t value2 = 0x5678;
  
  // Format string with padding: H4xH
  // This means: uint16, 4 bytes padding, uint16
  const char* format = "H4xH";
  
  Serial.println("Format string: \"H4xH\" (uint16, 4 bytes padding, uint16)");
  Serial.println("Note: Padding bytes are skipped and not modified");
  
  // Pack with padding
  Serial.println("Packing data with padding...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format, value1, value2);
  
  if (result == NULL) {
    Serial.println("Padding example packing failed");
    return;
  }
  
  // Display packed data including padding
  Serial.println("Packed data with padding (8 bytes total):");
  for (int i = 0; i < 8; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Highlight padding bytes
  Serial.println("Byte layout:");
  Serial.println("Bytes 0-1: First uint16 value (0x1234)");
  Serial.println("Bytes 2-5: Padding (should still be 0xAA)");
  Serial.println("Bytes 6-7: Second uint16 value (0x5678)");
  
  // Verify padding bytes are unchanged
  bool paddingUnchanged = true;
  for (int i = 2; i < 6; i++) {
    if (buffer[i] != 0xAA) {
      paddingUnchanged = false;
      break;
    }
  }
  
  Serial.print("Padding bytes unchanged: ");
  Serial.println(paddingUnchanged ? "Yes" : "No");
  
  // Unpack with padding
  uint16_t unpackedValue1, unpackedValue2;
  
  Serial.println("\nUnpacking data with padding...");
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, format, 
                                           &unpackedValue1, &unpackedValue2);
  
  if (unpackResult == NULL) {
    Serial.println("Padding example unpacking failed");
    return;
  }
  
  // Display unpacked values
  Serial.print("Unpacked value 1: 0x");
  Serial.println(unpackedValue1, HEX);
  
  Serial.print("Unpacked value 2: 0x");
  Serial.println(unpackedValue2, HEX);
}

/**
 * @brief Example 2: Complex data structures
 * 
 * Demonstrates how to pack and unpack complex data structures with mixed types
 */
void complexDataStructureExample(uint8_t* buffer) {
  Serial.println("\n--- Example 4: Complex Data Structures ---");
  
  // Define a complex data structure
  // - Header (uint16)
  // - Version (uint8)
  // - Flags (uint8)
  // - Timestamp (uint32)
  // - 3 temperature readings (3 x int8)
  // - 2 pressure readings (2 x uint16)
  // - Device ID string (10 bytes)
  // - Checksum (uint16)
  
  // Prepare data
  uint16_t header = 0xABCD;
  uint8_t version = 1;
  uint8_t flags = 0x03;  // Example: bit 0 = battery low, bit 1 = error
  uint32_t timestamp = millis();
  int8_t temperatures[3] = {22, 23, 21};  // °C
  uint16_t pressures[2] = {1013, 1012};   // hPa
  char deviceId[] = "ARDUINO001";
  uint16_t checksum = 0xFFFF;  // Dummy checksum
  
  // Format string
  const char* format = "HBBi3b2H10sH";
  
  // Pack the complex structure
  Serial.println("Packing complex data structure...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, format,
                             header, version, flags, timestamp,
                             temperatures, pressures, deviceId, checksum);
  
  if (result == NULL) {
    Serial.println("Complex structure packing failed");
    return;
  }
  
  // Calculate total size
  size_t totalSize = (uint8_t*)result - buffer;
  
  // Display packed data
  Serial.print("Packed data size: ");
  Serial.print(totalSize);
  Serial.println(" bytes");
  
  Serial.println("Packed data (hexadecimal):");
  for (size_t i = 0; i < totalSize; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
  
  // Unpack the complex structure
  uint16_t unpackedHeader;
  uint8_t unpackedVersion;
  uint8_t unpackedFlags;
  uint32_t unpackedTimestamp;
  int8_t unpackedTemperatures[3];
  uint16_t unpackedPressures[2];
  char unpackedDeviceId[11] = {0};  // 10 bytes + null terminator
  uint16_t unpackedChecksum;
  
  Serial.println("Unpacking complex data structure...");
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, format,
                                           &unpackedHeader, &unpackedVersion, &unpackedFlags,
                                           &unpackedTimestamp, unpackedTemperatures,
                                           unpackedPressures, unpackedDeviceId, &unpackedChecksum);
  
  if (unpackResult == NULL) {
    Serial.println("Complex structure unpacking failed");
    return;
  }
  
  // Display unpacked data
  Serial.println("Unpacked data:");
  Serial.print("Header: 0x");
  Serial.println(unpackedHeader, HEX);
  
  Serial.print("Version: ");
  Serial.println(unpackedVersion);
  
  Serial.print("Flags: 0x");
  Serial.println(unpackedFlags, HEX);
  
  Serial.print("Timestamp: ");
  Serial.println(unpackedTimestamp);
  
  Serial.print("Temperatures: ");
  for (int i = 0; i < 3; i++) {
    Serial.print(unpackedTemperatures[i]);
    Serial.print("°C");
    if (i < 2) Serial.print(", ");
  }
  Serial.println();
  
  Serial.print("Pressures: ");
  for (int i = 0; i < 2; i++) {
    Serial.print(unpackedPressures[i]);
    Serial.print(" hPa");
    if (i < 1) Serial.print(", ");
  }
  Serial.println();
  
  Serial.print("Device ID: ");
  Serial.println(unpackedDeviceId);
  
  Serial.print("Checksum: 0x");
  Serial.println(unpackedChecksum, HEX);
}

/**
 * @brief Example 3: Sensor data packet
 * 
 * Demonstrates a practical application: formatting sensor data into packets
 */
void sensorDataPacketExample(uint8_t* buffer) {
  Serial.println("\n--- Example 5: Sensor Data Packet ---");
  
  // Packet ID (increments with each transmission)
  static uint8_t packetId = 0;
  
  // Prepare mock sensor data
  int8_t temperatures[3] = {21, 22, 23};  // Temperature readings in °C
  uint16_t adcValues[4] = {512, 768, 1024, 1536};  // ADC readings
  float sensorReadings[2] = {98.6, 1013.25};  // Body temp (°F), Pressure (hPa)
  char deviceName[] = "SENSOR1";  // Device name
  
  // Pack the sensor data packet
  Serial.println("Packing sensor data packet...");
  void* result = CStruct::pack(buffer, BUFFER_SIZE, PACKET_FORMAT,
                             PACKET_HEADER, packetId,
                             temperatures, adcValues, sensorReadings, deviceName);
  
  if (result == NULL) {
    Serial.println("Sensor data packet packing failed");
    return;
  }
  
  // Calculate packet size
  size_t packetSize = (uint8_t*)result - buffer;
  
  // Display packet information
  Serial.print("Packet size: ");
  Serial.print(packetSize);
  Serial.println(" bytes");
  
  Serial.println("Packet data (hexadecimal):");
  for (size_t i = 0; i < packetSize; i++) {
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
  
  // In a real application, you would now send this packet via serial, radio, etc.
  Serial.println("In a real application, this packet would be transmitted");
  Serial.println("to another device via serial, radio, or other communication channel.");
  
  // Simulate receiving and unpacking the packet
  Serial.println("\nSimulating packet reception and unpacking...");
  
  // Variables for unpacking
  uint16_t unpackedHeader;
  uint8_t unpackedPacketId;
  int8_t unpackedTemperatures[3];
  uint16_t unpackedAdcValues[4];
  float unpackedSensorReadings[2];
  char unpackedDeviceName[8] = {0};  // 7 bytes + null terminator
  
  // Unpack the packet
  const void* unpackResult = CStruct::unpack(buffer, BUFFER_SIZE, PACKET_FORMAT,
                                           &unpackedHeader, &unpackedPacketId,
                                           unpackedTemperatures, unpackedAdcValues,
                                           unpackedSensorReadings, unpackedDeviceName);
  
  if (unpackResult == NULL) {
    Serial.println("Sensor data packet unpacking failed");
    return;
  }
  
  // Verify packet header
  if (unpackedHeader != PACKET_HEADER) {
    Serial.println("Error: Invalid packet header");
    return;
  }
  
  // Display unpacked sensor data
  Serial.println("Received sensor data:");
  Serial.print("Packet ID: ");
  Serial.println(unpackedPacketId);
  
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
  
  Serial.print("Body Temperature: ");
  Serial.print(unpackedSensorReadings[0], 1);
  Serial.println("°F");
  
  Serial.print("Atmospheric Pressure: ");
  Serial.print(unpackedSensorReadings[1], 2);
  Serial.println(" hPa");
  
  Serial.print("Device Name: ");
  Serial.println(unpackedDeviceName);
  
  // Increment packet ID for next transmission
  packetId++;
  if (packetId > 255) packetId = 0;
}
