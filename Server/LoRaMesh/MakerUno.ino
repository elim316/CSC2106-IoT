#include <SoftwareSerial.h>

#define RX_PIN 2
#define TX_PIN 3

SoftwareSerial uartSerial(RX_PIN, TX_PIN); // RX, TX

void setup() {
  Serial.begin(9600);
  uartSerial.begin(9600);
}

void loop() {
  // Read sensor data or perform other tasks
  // Example: int sensorData = analogRead(A0);
  
  // Send data via UART to Central M5Stick
  uartSerial.println("Sensor data");
  
  delay(1000);
}
