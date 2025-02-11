#include "wifi_setup.h"

// // WiFi Configuration
// const char* ssid = "gauer";
// const char* password = "adminadmin";
// const char* host = "192.168.4.1";
// const int port = 4300;

// const char* ssid = "Special Projects-5GHz";
// const char* password = "sprojects1!";
// const char* host = "192.168.1.156";
// const int port = 4300;

// WiFi Configuration
const char* ssid = "SparkRobotic";
const char* password = "weresometal";
const char* host = "192.168.1.238";
const int port = 4300;

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}
