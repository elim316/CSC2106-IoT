#include <SoftwareSerial.h>
#include <SPI.h>
#include <LoRa.h>

#define RX_PIN 2  // Define RX pin for UART communication
#define TX_PIN 3  // Define TX pin for UART communication

#define LORA_CS 10
#define LORA_RST 9
#define LORA_IRQ 8

SoftwareSerial uart(RX_PIN, TX_PIN); // RX, TX

void setup() {
  Serial.begin(9600);
  while (!Serial);

  uart.begin(9600);
  
  // LoRa setup
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  if (!LoRa.begin(433E6)) { // Initialize LoRa module with frequency (in Hz)
    Serial.println("LoRa initialization failed. Check your connections.");
    while (true);
  }
}

void loop() {
  if (uart.available()) {
    // Read data from overall building M5Stick
    String data = uart.readStringUntil('\n');
    sendDataToLoRaGateway(data);
  }
}

void sendDataToLoRaGateway(String data) {
  // Send data to LoRaGateway via LoRa
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
  Serial.println("Data sent to LoRaGateway: " + data);
}
