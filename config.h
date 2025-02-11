#ifndef CONFIG_H
#define CONFIG_H

// // WiFi Credentials
// const char* ssid = "gauer";
// const char* password = "adminadmin";

// // Socket.IO Server Config
// const char* host = "192.168.4.1";
// const int port = 4300;

// const char* ssid = "Special Projects-5GHz";
// const char* password = "sprojects1!";
// const char* host = "192.168.1.185";
// const int port = 4300;

#include <SocketIoClient.h>

// WiFi Configuration
extern const char* ssid;
extern const char* password;
extern const char* host;
extern const int port;

// Encoder Configurations
extern const int pulsesPerRevolution;
extern float wheelDiameterInches;
extern float shearDelay;
extern float inchesPerPulse;
extern volatile long encoderCount;
extern float travelDistanceInches;
extern float inputLength;
extern int inputQuantity;
extern int cutCount;

// Flags
extern bool feedingMaterial;
extern volatile bool isPaused;
extern bool cuttingMaterial;
extern volatile bool isReset;
extern bool manualOverride;
extern bool manualShearOverride;
extern bool isEStopActive;
extern volatile bool eStopResetRequired;
extern volatile bool isResumeRequired;

// Socket.IO Client
extern SocketIoClient socket; // Declare the socket object globally

#endif // CONFIG_H