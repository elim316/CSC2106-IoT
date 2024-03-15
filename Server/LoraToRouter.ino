#include <SPI.h>
#include <LoRa.h>

#define SENDER_GATEWAY_ID 1   // ID of the sender gateway
#define RECEIVER_GATEWAY_ID 2 // ID of the receiver gateway

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!LoRa.begin(433E6)) { // Initialize LoRa module with frequency (in Hz)
    Serial.println("LoRa initialization failed. Check your connections.");
    while (true);
  }
}

void loop() {
  // Send a message from the sender gateway
  sendMessage("Hello from Sender Gateway", RECEIVER_GATEWAY_ID);

  // Receive messages on the receiver gateway
  receiveMessage();
}

void sendMessage(String message, int receiverID) {
  LoRa.beginPacket();
  LoRa.print(receiverID); // Send receiver gateway ID
  LoRa.print(":");
  LoRa.print(message); // Send message
  LoRa.endPacket();
  Serial.println("Message sent: " + message);
  delay(1000); // Wait a bit before sending the next message
}

void receiveMessage() {
  if (LoRa.parsePacket()) {
    String message = "";

    // Read receiver gateway ID
    int receiverID = LoRa.parseInt();
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    Serial.print("Received message for Gateway ");
    Serial.print(receiverID);
    Serial.print(": ");
    Serial.println(message);
  }
}
