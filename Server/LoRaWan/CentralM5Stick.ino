//This file will handle the communication between the MakerUno/LoraHat and the M5Stick Plus via UART.

#include <SPI.h>

#define LORA_CS 10
#define LORA_RST 9
#define LORA_IRQ 8

#define RX_PIN 2  // Define RX pin for UART communication
#define TX_PIN 3  // Define TX pin for UART communication

SoftwareSerial uart(RX_PIN, TX_PIN); // RX, TX

void setup() {
  Serial.begin(9600);
  while (!Serial);

  uart.begin(9600);

  // Initialize SPI bus
  SPI.begin();

  // Setup LoRa module
  pinMode(LORA_CS, OUTPUT);
  digitalWrite(LORA_CS, HIGH); // Set CS pin high initially
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW); // Reset LoRa module
  delay(10);
  digitalWrite(LORA_RST, HIGH);

  // Wait for LoRa module to initialize
  delay(100);

  // Print initialization message
  Serial.println("LoRa initialization complete.");
}

void loop() {
  // Check for incoming data from MakerUno/LoraHat via LoRa
  if (receiveFromLoRa()) {
    String data = "";
    while (Serial.available()) {
      data += (char)Serial.read();
    }
    sendDataToNonCentralM5Stick(data);
  }

  // Check for incoming data from non-centralized M5Stick via UART
  if (uart.available()) {
    String data = uart.readStringUntil('\n');
    sendToLoRaGateway(data);
  }

  // Check for incoming data from LoRa gateway
  receiveFromLoRaGateway();

  // Check for incoming data from M5Stick Plus
  receiveFromM5StickPlus();
}

bool receiveFromLoRa() {
  if (digitalRead(LORA_IRQ) == LOW) {
    // Select LoRa module
    digitalWrite(LORA_CS, LOW);

    // Read data from LoRa module via SPI
    char c;
    while ((c = SPI.transfer(0)) != 's') {} // Wait for start byte
    String data = "";
    while ((c = SPI.transfer(0)) != 'e') { // Read until end byte
      data += c;
    }

    // Deselect LoRa module
    digitalWrite(LORA_CS, HIGH);

    // Process received data
    Serial.println("Data received from LoRaGateway: " + data);
    return true;
  }
  return false;
}

void sendDataToNonCentralM5Stick(String data) {
  // Send data to non-centralized M5Stick via UART
  uart.println(data);
  Serial.println("Data sent to non-centralized M5Stick: " + data);
}

void sendToLoRaGateway(String data) {
  // Select LoRa module
  digitalWrite(LORA_CS, LOW);

  // Send data via SPI
  SPI.transfer('s'); // Send start byte
  for (char c : data) {
    SPI.transfer(c);
  }
  SPI.transfer('e'); // Send end byte

  // Deselect LoRa module
  digitalWrite(LORA_CS, HIGH);

  Serial.println("Data sent to LoRa gateway: " + data);
}

void receiveFromLoRaGateway() {
  // Check if data is available from LoRa module
  if (Serial.available()) {
    String data = "";
    while (Serial.available()) {
      data += (char)Serial.read();
    }
    // Process received data from LoRaGateway if needed
    Serial.println("Data received from LoRaGateway: " + data);
  }
}

void receiveFromM5StickPlus() {
  // Check if data is available from M5Stick Plus via UART
  if (Serial.available()) {
    String data = Serial.readString();
    // Process received data from M5Stick Plus if needed
    Serial.println("Data received from M5Stick Plus: " + data);
  }
}


// #include <SoftwareSerial.h>
// #include <SPI.h>
// #include <LoRa.h>

// #define RX_PIN 2  // Define RX pin for UART communication
// #define TX_PIN 3  // Define TX pin for UART communication

// #define LORA_CS 10
// #define LORA_RST 9
// #define LORA_IRQ 8

// SoftwareSerial uart(RX_PIN, TX_PIN); // RX, TX

// void setup() {
//   Serial.begin(9600);
//   while (!Serial);

//   uart.begin(9600);
  
//   // LoRa setup
//   LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
//   if (!LoRa.begin(433E6)) { // Initialize LoRa module with frequency (in Hz)
//     Serial.println("LoRa initialization failed. Check your connections.");
//     while (true);
//   }
// }

// void loop() {
//   if (uart.available()) {
//     // Read data from overall building M5Stick
//     String data = uart.readStringUntil('\n');
//     sendDataToLoRaGateway(data);
//   }
//   receiveFromLoRaGateway();

//   if (Serial.available()) {
//     String data = Serial.readString();
//     sendDataToM5StickPlus(data);
//   }
//   receiveFromM5StickPlus();
// }

// void sendDataToLoRaGateway(String data) {
//   // Send data to LoRaGateway via LoRa
//   LoRa.beginPacket();
//   LoRa.print(data);
//   LoRa.endPacket();
//   Serial.println("Data sent to LoRaGateway: " + data);
// }

// void sendToM5StickPlus(String data) {
//   Serial.write(data.c_str());
// }

// void receiveFromLoRaGateway() {
//   // Check if data is available from LoRa module
//   if (LoRa.parsePacket()) {
//     String data = "";
//     while (LoRa.available()) {
//       data += (char)LoRa.read();
//     }
//     Serial.println("Data received from LoRaGateway: " + data);
//     // Process received data from LoRaGateway if needed
//   }
// }

// void receiveFromM5StickPlus() {
//   // Check if data is available from M5Stick Plus via UART
//   if (Serial.available()) {
//     String data = Serial.readString();
//     // Process received data from M5Stick Plus if needed
//     Serial.println("Data received from M5Stick Plus: " + data);
//   }
// }