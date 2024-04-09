#include <WiFi.h>
#include <WebServer.h>
#include <M5StickCPlus.h>
#include <ArduinoJson.h>
#include "time.h"

// Serial
#define SERIAL_RX 0
#define SERIAL_TX 25
#define LORA_BAUD_RATE 115200

#define timeZone 32400
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

// Function Prototype
void handleRouteNotFound();
void handleToggleLights();
void handleGetData();
void setupWifi();
void setupSyncTimeServer();


// Global variable
const char* ssid = "Xiaomi 13T";
const char* password =  "Trickster123";

// NTP Server that is used for timesync
char* ntpServer =  "sg.pool.ntp.org";

// Standardized message
struct Message {
  uint8_t destID;
  uint8_t payload[50]; 
  uint8_t checksum;
};

WebServer httpServer(80);

// We transfer the Serial to 
void setup() {

    M5.begin();

    // Setup WiFi (E.g. WIFI Connection)
    setupWifi();

    // Setup NTP Timing
    setupSyncTimeServer();

    // Setup UART transmission to MainLoRaNode (We can use Serial1 to send data over to the LoRaMainNode)
    Serial1.begin(LORA_BAUD_RATE, SERIAL_8N1, SERIAL_RX, SERIAL_TX);

    // Setup event handlers for server.
    // Upon receiving command from the httpServer
    // Send it through UART(Serial1) to LoRaWAN respective deviceID (E.g. DeviceID=2 with action)
    httpServer.on("/get-data", handleGetData);
    httpServer.on("/toggle-light", handleToggleLights);

    // Setup Server
    httpServer.begin();
    Serial.println("HTTP Server started");
    Serial.println("Connected to the WiFi network. IP Address: ");
    Serial.println(WiFi.localIP());
}

void handleToggleLights() {
    // Send to the LoRa via UART
    String data = "TOGGLIGHTS\n";
    Serial1.write(data.c_str());
    
//   if (request->hasParam("deviceID")) {

//     // Send this request to MainLoRaNode for it to send it to the respective LoRaNode (Child)
//     // By doing this, we are assuming that the server is always connected to the main LoRaNode
//     // Example data to be sent
//     // 1:TOGGLELIGHT\n
//     // Where 1 is the deviceID and TOGGLELIGHT is the command
//     // Basically telling deviceID to toggle the lights on
//     String data = String(request->getParam("deviceID", true)->value()) + ":" + "TOGGLELIGHT" + "\n";
//     // Send the data over to the Main LoraNode
//     Serial1.write(data.c_str());
//   }
}

void handleRouteNotFound() {
    JsonDocument doc;
    doc["message"] = "Route not found";

    String jsonOutput;
    serializeJson(doc, jsonOutput);

    httpServer.send(404, "application/json", jsonOutput);
}

void handleGetData() {
    M5.Lcd.printf("Received data connection\n");
    Serial1.write("Received battle of Yutong\n");

    httpServer.send(200, "text/plain", "Received data connection");
}

void setupSyncTimeServer() {
    // Set ntp time to local
    configTime(timeZone, 0, ntpServer);

    // Get local time
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
        // Set RTC time
        RTC_TimeTypeDef TimeStruct;
        TimeStruct.Hours   = timeInfo.tm_hour;
        TimeStruct.Minutes = timeInfo.tm_min;
        TimeStruct.Seconds = timeInfo.tm_sec;
        M5.Rtc.SetTime(&TimeStruct);

        RTC_DateTypeDef DateStruct;
        DateStruct.WeekDay = timeInfo.tm_wday;
        DateStruct.Month = timeInfo.tm_mon + 1;
        DateStruct.Date = timeInfo.tm_mday;
        DateStruct.Year = timeInfo.tm_year + 1900;
        M5.Rtc.SetDate(&DateStruct);
        Serial.println("Time is now matching NTP");
    }
}

void setupWifi() {
    delay(10);
    M5.Lcd.printf("Connecting to %s", ssid);
    WiFi.mode(WIFI_STA);        // Set the mode to WiFi station mode.
    WiFi.begin(ssid, password); // Start Wifi connection.

    M5.Lcd.printf("Initialising Group30-M5StickAPI", 0);

    // Setting the hostname
    WiFi.setHostname("Group30-M5StickAPI");

    Serial.print("Starting WiFI connection");
    while (WiFi.status() != WL_CONNECTED)
    {
    delay(500);
    M5.Lcd.print(".");
    Serial.println("Attempting to connect to WiFi");
    }
    M5.Lcd.print("IP: ");
    M5.Lcd.println(WiFi.localIP());
}

void loop() {
    // Server refers to API
    httpServer.handleClient();

    if (RTC_TimeStruct.Hours == 8) {
        String data = "TOGGLIGHTS\n";
        Serial1.write(data.c_str());
    }

    if (RTC_TimeStruct.Hours == 20) {
        String data = "TOGGLIGHTS\n";
        Serial1.write(data.c_str());
    }
}
