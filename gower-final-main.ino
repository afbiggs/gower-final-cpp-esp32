#include <Arduino.h>
// #include <SocketIoClient.h>
#include "config.h"
#include "pins.h"
#include "hardware_setup.h"
#include "wifi_setup.h"
#include "socket_handlers.h"
#include "utility.h"
#include "config.h"

// SocketIoClient socket;
// // #include "global_vars.h"

// const char* ssid = "Special Projects-5GHz";
// const char* password = "sprojects1!";
// const char* host = "192.168.1.185";
// const int port = 4300;

// // WiFi Configuration
// const char* ssid = "gauer";
// const char* password = "adminadmin";
// const char* host = "192.168.4.1";
// const int port = 4300;

// Encoder Configurations
const int pulsesPerRevolution = 1600;
float wheelDiameterInches = 1.885;
float inchesPerPulse = (3.14159265 * wheelDiameterInches) / pulsesPerRevolution;
float shearDelay = 6000;

// Global Variables
// volatile long encoderCount = 0;
float travelDistanceInches = 0.0;
float inputLength = 0;
int inputQuantity = 0;
int cutCount = 0;



bool feedingMaterial = false;
volatile bool isPaused = false;
bool cuttingMaterial = false;
volatile bool isReset = false;
bool manualOverride = false;
bool manualShearOverride = false;
bool isEStopActive = false;
volatile bool eStopResetRequired = false;
volatile bool isResumeRequired = false;



// Socket.IO Client
SocketIoClient socket; // Define the socket object


void setup() {
    Serial.begin(115200);
    initializeRelays();
    initializeLimitSwitches();
    initializeEncoders();
    connectToWiFi();
    configureSocketIOHandlers();
    socket.begin(host, port);
}

// bool isConnected = false;

// void setup() {
//     Serial.begin(115200);
//     Serial.println("Ready to receive JSON...");

//     // Initialize all hardware and connections
//     initializeRelays();
//     initializeLimitSwitches();
//     initializeEncoders();
//     connectToWiFi();
//     configureSocketIOHandlers(socket);

//      // Start Socket.IO communication
//     socket.on("connect", [](const char* payload, size_t length) {
//         Serial.println("[SIoC] Connected to server.");
//         isConnected = true; // Update connection state
//     });

//     socket.on("disconnect", [](const char* payload, size_t length) {
//         Serial.println("[SIoC] Disconnected from server.");
//         isConnected = false; // Update connection state
//     });

//     socket.begin(host, port);
//     Serial.println("Setup completed.");
// }


void loop() {
    static unsigned long lastEmitTime = 0;
    unsigned long currentTime = millis();

    // E-Stop or Resume Handling
    if (isEStopActive || isResumeRequired) {
        stopAllRelays();
        socket.loop();
        delay(5);
        return;
    }

    // Manual Overrides
    if (manualOverride || manualShearOverride) {
        handleManualOverrides();
        socket.loop();
        delay(5);
        return;
    }

    // Core Cutting Process
    if (!isPaused && !isReset && inputQuantity > 0) {
        handleCuttingProcess();
    } else {
        stopAllRelays();
    }

    // Reset Handling
    if (isReset) {
        handleResetState();
        return;
    }

    // Emit Travel Distance Every 50ms
    if (currentTime - lastEmitTime >= 50) {
        lastEmitTime = currentTime;
        sendTravelDistance();
    }

    socket.loop();
    delay(5);
}

// void loop() {
//     static unsigned long lastEmitTime = 0;
//     unsigned long currentTime = millis();

//     // Check for E-Stop or Resume Requirement
//     if (isEStopActive || isResumeRequired) {
//         digitalWrite(materialForwardRelay, LOW);
//         digitalWrite(manualShearRelay, LOW);
//         cuttingMaterial = false;
//         feedingMaterial = false;
//         socket.loop();
//         // readSerialData();
//         delay(5);
//         return; // Skip further processing
//     }

//     // Handle Manual Overrides
//     if (manualOverride || manualShearOverride) {
//         if (manualOverride) {
//             digitalWrite(materialForwardRelay, HIGH);
//         } else {
//             digitalWrite(materialForwardRelay, LOW);
//         }
//         if (manualShearOverride) {
//             digitalWrite(manualShearRelay, HIGH);
//         } else {
//             digitalWrite(manualShearRelay, LOW);
//         }
//         socket.loop();
//         // readSerialData();
//         delay(5);
//         return; // Skip the rest of the loop
//     }

//     // Core Cutting Process
//     if (!isPaused && !isReset && inputQuantity > 0) {
//         travelDistanceInches = encoderCount * inchesPerPulse;

//         if (travelDistanceInches < inputLength) {
//             digitalWrite(materialForwardRelay, HIGH); // Continue feeding material
//             cuttingMaterial = true;
//         } else {
//             digitalWrite(materialForwardRelay, LOW); // Stop feeding material
//             Serial.println("Target length reached. Material feed stopped.");

//             // Start Shear Process
//             digitalWrite(manualShearRelay, HIGH);
//             Serial.println("Shear process started.");
//             sleep(10);

//             digitalWrite(manualShearRelay, LOW);

//             // // Wait for Shear End Switch
//             // while (digitalRead(shearEndSwitch) == HIGH) {
//             //     delay(10);
//             // }
//             // Serial.println("Shear End Switch triggered. Shear process complete.");
//             // digitalWrite(manualShearRelay, LOW);

//             // // Wait for Shear Start Switch
//             // Serial.println("Waiting for Shear Start Switch...");
//             // while (digitalRead(shearStartSwitch) == HIGH) {
//             //     delay(10);
//             // }
//             Serial.println("Shear Start Switch triggered. Starting material feed...");

//             // Increment Cut Count and Reset Encoder
//             cutCount++;
//             sendCutCount();

//             if (isReset) {
//                 inputQuantity = 0;
//                 cuttingMaterial = false;
//                 Serial.println("Reset triggered after completing current cut.");
//                 return;
//             }

//             inputQuantity--;
//             encoderCount = 0;
//             travelDistanceInches = 0.0;
//             cuttingMaterial = false;
//         }
//     } else {
//         digitalWrite(materialForwardRelay, LOW);
//         digitalWrite(manualShearRelay, LOW);
//         cuttingMaterial = false;
//     }

//     // Handle Reset State
//     if (isReset) {
//         if (cuttingMaterial || (inputQuantity > 0 && !isPaused)) {
//             Serial.println("Cannot reset while cutting or feeding material. Pause the machine first.");
//             isReset = false; // Clear the reset request to avoid infinite loop
//             return; // Exit the loop early
//         } else {
//             // Perform reset when the machine is paused
//             Serial.println("Resetting machine...");
//             digitalWrite(materialForwardRelay, LOW);
//             digitalWrite(manualShearRelay, LOW);
//             encoderCount = 0;
//             travelDistanceInches = 0.0;
//             inputLength = 0;
//             inputQuantity = 0;
//             cutCount = 0;
//             isPaused = false; // Ensure paused state is cleared
//             cuttingMaterial = false; // Ensure no cutting is in progress
//             isReset = false; // Clear reset state
//             Serial.println("System reset completed. Ready for new parameters.");
//         }
//         return; // Exit the loop early to prevent further processing
//     }

//     // Emit Travel Distance Every 50ms
//     if (currentTime - lastEmitTime >= 50) {
//         lastEmitTime = currentTime;
//         sendTravelDistance();
//     }

//     socket.loop();
//     // readSerialData();
//     delay(5);
// }