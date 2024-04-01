#include <M5StickCPlus.h>

// Serial Pins
#define SERIAL_RX 0
#define SERIAL_TX 25

// Ultrasonic Pins
#define TRIG 33
#define ECHO 32

// Light Pins
#define lightPin 26

// Function Prototype
void setupLights(int pin);
void setupUltrasonic(int triggerPin, int echoPin);
int getDistance(int triggerPin, int echoPin);
void toggleLight(int pin, uint8_t value);
int getDistance(int triggerPin, int echoPin);


// Global variables (Setup current light value here?)

void setupUltrasonic(int triggerPin, int echoPin) {
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(triggerPin, LOW);
}

void setupLights(int pin) {
  pinMode(pin, OUTPUT);
}

int getDistance(int triggerPin, int echoPin) {
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int dist = duration * 0.017;
  return dist;
}

void toggleLight(int pin, uint8_t value) {
  digitalWrite(pin, value);
}

void setup() {
  M5.begin();
  // Setup Serial Connection
  Serial1.begin(115200, SERIAL_8N1, SERIAL_RX, SERIAL_TX);

  // Setup initial pins connection
  setupUltrasonic(TRIG, ECHO);
  setupLights(lightPin);

  // Testing Purposes for lightPin
  toggleLight(lightPin, LOW);
}

void loop() {
  int distance = getDistance(TRIG, ECHO);

  String data = "Distance: " + String(distance) + " cm\n";
  Serial.println(data);
  Serial.println("Sending data through UART");

  // Sent data to UART
  Serial1.write(data.c_str());


  delay(1000);
}
