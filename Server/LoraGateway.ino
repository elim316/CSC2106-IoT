#include <SPI.h>
#include <LoRa.h>

#define LORA_CS 10
#define LORA_RST 9
#define LORA_IRQ 8

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // LoRa setup
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  if (!LoRa.begin(433E6)) { // Initialize LoRa module with frequency (in Hz)
    Serial.println("LoRa initialization failed. Check your connections.");
    while (true);
  }
}

void loop() {
  if (LoRa.parsePacket()) {
    // Read data from MakerUno
    String data = "";
    while (LoRa.available()) {
      data += (char)LoRa.read();
    }
    processData(data);
  }
}

void processData(String data) {
  // Process the received data
  Serial.println("Data received from MakerUno: " + data);
  // Add your code here to handle the received data as needed
}
