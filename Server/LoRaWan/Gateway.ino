// This code is for use on the Gateway 
#include <RH_RF95.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

#define RF95_FREQ 433.0 // LoRa frequency (in MHz), adjust as necessary

RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Message structure
struct Message {
  uint8_t destID;
  uint8_t payload[50];
  uint8_t checksum;
};

void sendWithRetransmit(const Message& msg) {
  uint8_t retries = 3; // no of attempts

  // Sending message 
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sending message");
  display.display();
  delay(1000);

  while (retries > 0) {
    rf95.send((uint8_t*)&msg, sizeof(msg));
    rf95.waitPacketSent();

    // Wait for message
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Waiting for message");
    display.display();

    // Wait for ACK
    if (rf95.waitAvailableTimeout(1000)) {
      Message ack;
      uint8_t len = sizeof(ack);
      if (rf95.recv((uint8_t*)&ack, &len)) {
        if (ack.destID == 2 && ack.checksum == msg.checksum) {
          // Received ACK from receiver
          Serial.println("Message sent successfully!");
          Serial.println((char*)ack.payload);
          // Received message 
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Received Message");
          display.display();
          delay(1000);
          return;
        }
      }
    }

    // Retransmit on failure
    retries--;
    Serial.println("Retransmitting...");
  }

  Serial.println("Failed to send message after multiple retries!");
}

void sendAck(const Message &msg) {
  Message ack;
  ack.destID = 2;
  strcpy((char *)ack.payload, "Hello, this is from Gateway");
  ack.checksum = msg.checksum;
  rf95.send((uint8_t *)&ack, sizeof(ack));
  rf95.waitPacketSent();
}

void displaySetup() {
  // Setup OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("LoRa Transceiver");
  display.display();
  delay(2000);
}

void setup() {
  Serial.begin(9600);

  displaySetup();

  // Initialize LoRa module
  if (!rf95.init()) {
    display.println("LoRa initialization failed!");
    display.display();
    while (1)
      ;
  }
  display.println("LoRa initialization OK!");
  display.display();

  if (!rf95.setFrequency(RF95_FREQ)) {
    display.println("Error setting frequency!");
    display.display();
    while (1)
      ;
  }
}

void loop() {
  if (Serial.available()) {
    // Transmitting from MakerUno/LoraHat to LoRa Gateway
    String data = Serial.readString();
    Message msg;
    msg.destID = 1; // Adjust destination ID as needed
    strcpy((char *)msg.payload, data.c_str()); // Adjust payload as needed
    sendWithRetransmit(msg);
  }

  if (rf95.available()) {
    // Receiving from LoRa Gateway to MakerUno/LoraHat
    Message msg;
    uint8_t len = sizeof(msg);

    // Waiting for Message
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Waiting for message");
    display.display();
    delay(1000);

    if (rf95.recv((uint8_t *)&msg, &len)) {
      if (validateChecksum(msg)) {
        // Check if the message is intended for the targeted device
        if (msg.destID == 1) {
          Serial.print("Received: ");
          Serial.println((char *)msg.payload);

          // Received Message
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Received message");
          display.display();
          delay(1000);

          // Sending ACK back to the sender
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Sending ACK");
          display.display();
          delay(1000);
          sendAck(msg);
        }
      } else {
        Serial.println("Checksum validation failed!");
      }
    }
  }
}

// To ensure no error
bool validateChecksum(const Message &msg) {
  uint8_t calculatedChecksum = 0;
  for (int i = 0; i < sizeof(msg) - 1; i++) {
    calculatedChecksum ^= *((uint8_t *)&msg + i);
  }
  return (calculatedChecksum == msg.checksum);
}
