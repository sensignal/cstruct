/* =========================================================================
    CStruct; binary pack/unpack tools for Arduino - Sensor Data Packet Example
    Copyright (c) 2025 Sensignal Co.,Ltd.
    SPDX-License-Identifier: Apache-2.0
========================================================================= */

/**
 * @file SensorDataPacket.ino
 * @brief Example of packing/unpacking sensor data as binary packets
 * 
 * This sample demonstrates how to efficiently pack data from multiple sensors
 * into binary packets for serial transmission. It also implements the unpacking
 * process on the receiving end.
 */

#include <CStruct.h>

// Packet format definition
#define PACKET_HEADER 0xAA55  // Packet header
#define PACKET_FORMAT "<HBIhhHe"  // Format: Header(H), PacketID(B), Timestamp(I), 
                                  // AccelX,Y(h,h), Pressure(H), Temperature(e)

// Buffer size definition
#define BUFFER_SIZE 32

// Global variables
uint8_t packetBuffer[BUFFER_SIZE];  // Packet buffer
uint8_t packetId = 0;               // Packet ID (increments with each transmission)

// Mock sensor data
int16_t accelX, accelY;       // Accelerometer values
uint16_t pressure;            // Pressure sensor value
float temperature;            // Temperature sensor value

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for USB connection only)
  }
  
  Serial.println("CStruct Sensor Data Packet Example");
  Serial.println("==================================");
  
  // Initialize random seed (for mock data generation)
  randomSeed(analogRead(0));
}

void loop() {
  // Generate mock sensor data
  generateMockSensorData();
  
  // Pack the data
  packSensorData();
  
  // Send the packet (via serial)
  sendPacket();
  
  // Receive and unpack the packet (in a real application, sending and receiving would be on separate devices)
  receiveAndUnpackPacket();
  
  // Update packet ID
  packetId++;
  if (packetId > 255) packetId = 0;
  
  // Wait until next cycle
  delay(1000);
}

/**
 * @brief Generate mock sensor data
 */
void generateMockSensorData() {
  // Accelerometer (-2g to +2g, assuming 16384 = 1g)
  accelX = random(-16384, 16384);
  accelY = random(-16384, 16384);
  
  // Pressure sensor (950 to 1050 hPa, multiplied by 10 for integer conversion)
  pressure = random(9500, 10500);
  
  // Temperature sensor (-10 to 40°C)
  temperature = random(-100, 400) / 10.0;
  
  // Display raw data
  Serial.println("\n--- Raw Sensor Data ---");
  Serial.print("Packet ID: "); Serial.println(packetId);
  Serial.print("Timestamp: "); Serial.println(millis());
  Serial.print("Accel X: "); Serial.print(accelX); Serial.println(" (1/16384 g)");
  Serial.print("Accel Y: "); Serial.print(accelY); Serial.println(" (1/16384 g)");
  Serial.print("Pressure: "); Serial.print(pressure / 10.0); Serial.println(" hPa");
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
}

/**
 * @brief Pack sensor data into a binary packet
 */
void packSensorData() {
  // Initialize buffer
  memset(packetBuffer, 0, BUFFER_SIZE);
  
  // Get current timestamp
  uint32_t timestamp = millis();
  
  // Pack the data
  void* result = CStruct::pack(packetBuffer, BUFFER_SIZE, PACKET_FORMAT,
                              PACKET_HEADER, packetId, timestamp,
                              accelX, accelY, pressure, temperature);
  
  if (result == NULL) {
    Serial.println("Packing failed");
    return;
  }
  
  // Display packed binary data
  Serial.println("\n--- Packed Binary Data ---");
  Serial.print("Size: "); Serial.print(result - (void*)packetBuffer); Serial.println(" bytes");
  Serial.print("Data (hexadecimal): ");
  
  for (int i = 0; i < 15; i++) {  // 2 + 1 + 4 + 2 + 2 + 2 + 2 = 15 bytes
    if (packetBuffer[i] < 0x10) Serial.print("0");
    Serial.print(packetBuffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

/**
 * @brief Send packet via serial
 */
void sendPacket() {
  // Calculate packet size (in a real application, you would also consider header size)
  size_t packetSize = 15;  // Size based on the format
  
  // Send the packet
  Serial.println("\n--- Packet Transmission ---");
  Serial.write(packetBuffer, packetSize);
  Serial.println(); // For readability
}

/**
 * @brief Receive and unpack a packet
 * 
 * Note: In a real application, this function would run on the receiving device.
 * In this sample, we're doing both sending and receiving on the same device,
 * so the receiving process is simulated.
 */
void receiveAndUnpackPacket() {
  // In a real application, you would read data from serial here
  // In this sample, we already have the data in the buffer, so we unpack directly
  
  // Variables for unpacking
  uint16_t header;
  uint8_t recvPacketId;
  uint32_t timestamp;
  int16_t recvAccelX, recvAccelY;
  uint16_t recvPressure;
  float recvTemperature;
  
  // Unpack the data
  Serial.println("\n--- Packet Reception and Unpacking ---");
  const void* unpackResult = CStruct::unpack(packetBuffer, BUFFER_SIZE, PACKET_FORMAT,
                                           &header, &recvPacketId, &timestamp,
                                           &recvAccelX, &recvAccelY, &recvPressure, &recvTemperature);
  
  if (unpackResult == NULL) {
    Serial.println("Unpacking failed");
    return;
  }
  
  // Verify header
  if (header != PACKET_HEADER) {
    Serial.println("Invalid packet header");
    return;
  }
  
  // Display unpacked data
  Serial.println("Unpacked Sensor Data:");
  Serial.print("Packet ID: "); Serial.println(recvPacketId);
  Serial.print("Timestamp: "); Serial.println(timestamp);
  Serial.print("Accel X: "); Serial.print(recvAccelX); Serial.println(" (1/16384 g)");
  Serial.print("Accel Y: "); Serial.print(recvAccelY); Serial.println(" (1/16384 g)");
  Serial.print("Pressure: "); Serial.print(recvPressure / 10.0); Serial.println(" hPa");
  Serial.print("Temperature: "); Serial.print(recvTemperature); Serial.println(" °C");
}
