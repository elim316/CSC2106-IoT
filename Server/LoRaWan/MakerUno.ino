// This file will handle the communication between the LoRa gateway and the MakerUno/LoraHat.

#include <SPI.h>

#define LORA_CS 10
#define LORA_RST 9
#define LORA_IRQ 8

void setup() {
  Serial.begin(9600);
  while (!Serial);

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
  if (Serial.available()) {
    String data = Serial.readString();
    sendToLoRaGateway(data);
  }
  // Receive data from LoRa module
  receiveFromLoRaGateway();

  if (Serial.available()) {
    String data = Serial.readString();
    sendToCentralM5Stick(data);
  }
  receiveFromCentralM5Stick();
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
    processData(data);
  }
}

// CAN BE DONE ON FRONTEND
// void processData(String data) {
//   // Process the received data
//   Serial.println("Data received from LoRa gateway: " + data);

//   // Example: Parse received data and take action based on its content
//   if (data.startsWith("LED_ON")) {
//     // Turn on an LED or perform some action
//     digitalWrite(LED_BUILTIN, HIGH);
//     Serial.println("LED turned ON");
//   } else if (data.startsWith("LED_OFF")) {
//     // Turn off an LED or perform some action
//     digitalWrite(LED_BUILTIN, LOW);
//     Serial.println("LED turned OFF");
//   } else {
//     // If the received data does not match any known command, do nothing
//     Serial.println("Unknown command received");
//   }
// }

void sendToCentralM5Stick(String data) {
  // Send data to centralM5Stick via Serial
  Serial.println("Data sent to centralM5Stick: " + data);
}

void receiveFromCentralM5Stick() {
  // Check if data is available from centralM5Stick via Serial
  if (Serial.available()) {
    String data = Serial.readString();
    // Process received data from centralM5Stick
    Serial.println("Data received from centralM5Stick: " + data);
  }
}

// DELETE ON CODE CLEANUP

// #include <SPI.h>
// #include <LoRa.h>

// #define LORA_CS 10
// #define LORA_RST 9
// #define LORA_IRQ 8

// void setup() {
//   Serial.begin(9600);
//   while (!Serial);

//   // LoRa setup
//   LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
//   if (!LoRa.begin(433E6)) { // Initialize LoRa module with frequency (in Hz)
//     Serial.println("LoRa initialization failed. Check your connections.");
//     while (true);
//   }
// }

// void loop() {
//   if (Serial.available()) {
//     String data = Serial.readString();
//     sendToLoRaGateway(data);
//   }
//   if (LoRa.parsePacket()) {
//     String data = "";
//     while (LoRa.available()) {
//       data += (char)LoRa.read();
//     }
//     processLoRaData(data);
//   }
// }

// void processData(String data) {
//   // Process the received data
//   Serial.println("Data received from MakerUno: " + data);
//   // Add your code here to handle the received data as needed
// }

// void sendToLoRaGateway(String data) {
//   LoRa.beginPacket();
//   LoRa.print(data);
//   LoRa.endPacket();
// }
