#ifndef UTILITY_H
#define UTILITY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"


void handleResetState();

void sendTravelDistance();
void sendCutCount();
void stopAllRelays();
void handleManualOverrides();
void handleCuttingProcess();
void sendCutCount(); // Example of existing function
// String formatCycleTime(); // Declare the formatCycleTime function


#endif








// #ifndef UTILITY_H
// #define UTILITY_H

// #include <Arduino.h>
// #include "config.h"
// #include "hardware_setup.h"
  
// // Forward declaration of functions
// void sendCutCount(); // Add forward declaration
// void sendTravelDistance();
// void handleCuttingProcess();
// void handleResetState();
// void handleManualOverrides();

// // Send cut count to the server
// void sendCutCount() {
//     StaticJsonDocument<128> doc;
//     doc["cutCount"] = cutCount;

//     String output;
//     serializeJson(doc, output);
//     socket.emit("cut_status", output.c_str()); // Emit using the global socket object

//     Serial.print("Sent cut count to server: ");
//     Serial.println(output);
// }

// // Send travel distance to the server
// void sendTravelDistance() {
//     StaticJsonDocument<128> doc;
//     doc["travelDistance"] = encoderCount * inchesPerPulse;

//     String output;
//     serializeJson(doc, output);
//     socket.emit("travel_distance", output.c_str()); // Emit using the global socket object

//     Serial.print("Sent travel distance to server: ");
//     Serial.println(output);
// }

// // Handle cutting process
// void handleCuttingProcess() {
//     travelDistanceInches = encoderCount * inchesPerPulse;

//     if (travelDistanceInches >= inputLength) {
//         stopAllRelays();
//         Serial.println("Target length reached. Material feed stopped.");

//         // Start Shear Process
//         digitalWrite(manualShearRelay, HIGH);
//         Serial.println("Shear process started.");
//         delay(10); // Simulate shear process timing
//         digitalWrite(manualShearRelay, LOW);

//         // Increment cut count and reset encoder
//         cutCount++;
//         sendCutCount(); // Now properly declared and implemented

//         if (isReset) {
//             inputQuantity = 0;
//             cuttingMaterial = false;
//             Serial.println("Reset triggered after completing current cut.");
//             return;
//         }

//         // Decrement remaining quantity
//         inputQuantity--;
//         encoderCount = 0;
//         travelDistanceInches = 0.0;
//         cuttingMaterial = false;
//     } else {
//         digitalWrite(materialForwardRelay, HIGH); // Continue feeding material
//         cuttingMaterial = true;
//     }
// }

// // Handle reset state
// void handleResetState() {
//     stopAllRelays();
//     encoderCount = 0;
//     travelDistanceInches = 0.0;
//     inputLength = 0;
//     inputQuantity = 0;
//     cutCount = 0;
//     isPaused = false;
//     isReset = false;
//     cuttingMaterial = false;
//     Serial.println("System reset completed. Ready for new parameters.");
// }

// // Handle manual overrides
// void handleManualOverrides() {
//     if (manualOverride) {
//         digitalWrite(materialForwardRelay, HIGH);
//     } else {
//         digitalWrite(materialForwardRelay, LOW);
//     }

//     if (manualShearOverride) {
//         digitalWrite(manualShearRelay, HIGH);
//     } else {
//         digitalWrite(manualShearRelay, LOW);
//     }
// }

// #endif // UTILITY_H
