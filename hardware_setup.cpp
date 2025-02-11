#include "hardware_setup.h"
#include "pins.h"
#include "utility.h" // For sendCutCount()
#include "calibration.h" // Include the calibration module
#include <Arduino.h>


volatile long encoderCount = 0;
volatile unsigned long lastInterruptTimeA = 0;
volatile unsigned long lastInterruptTimeB = 0;
const unsigned long debounceDelay = 50;

// const int materialForwardRelay = 15;
// const int manualShearRelay = 2;
// const int shearStartSwitch = 27;
// const int shearEndSwitch = 33;
// const int encoderPinA = 17;
// const int encoderPinB = 34;

// extern bool manualOverride, manualShearOverride;
// // extern int materialForwardRelay, manualShearRelay;
// extern bool cuttingMaterial, isPaused, isReset;
// extern int encoderCount, inputQuantity;
// extern float inputLength, travelDistanceInches;
// extern float inchesPerPulse;
// extern int cutCount;


// volatile long encoderCount = 0;


void initializeRelays() {
    pinMode(materialForwardRelay, OUTPUT);
    digitalWrite(materialForwardRelay, LOW);
    pinMode(manualShearRelay, OUTPUT);
    digitalWrite(manualShearRelay, LOW);
    Serial.println("Relays initialized.");
}

void initializeLimitSwitches() {
    pinMode(shearStartSwitch, INPUT_PULLUP);
    pinMode(shearEndSwitch, INPUT_PULLUP);
    Serial.println("Limit switches initialized.");
}

void initializeEncoders() {
    pinMode(encoderPinA, INPUT);
    pinMode(encoderPinB, INPUT);

    attachInterrupt(digitalPinToInterrupt(encoderPinA), handleEncoderA, CHANGE);
    attachInterrupt(digitalPinToInterrupt(encoderPinB), handleEncoderB, CHANGE);

    Serial.println("Encoder pins initialized with debouncing.");
}

void handleEncoderA() {
    unsigned long currentTime = micros();
    if (currentTime - lastInterruptTimeA > debounceDelay) {
        bool A = digitalRead(encoderPinA);
        bool B = digitalRead(encoderPinB);
        if (A == B) {
            encoderCount++; // Clockwise
        } else {
            encoderCount--; // Counter-clockwise
        }
        lastInterruptTimeA = currentTime; // Update last interrupt time for PinA
    }
}

void handleEncoderB() {
    unsigned long currentTime = micros();
    if (currentTime - lastInterruptTimeB > debounceDelay) {
        bool A = digitalRead(encoderPinA);
        bool B = digitalRead(encoderPinB);
        if (A != B) {
            encoderCount++; // Clockwise
        } else {
            encoderCount--; // Counter-clockwise
        }
        lastInterruptTimeB = currentTime; // Update last interrupt time for PinB
    }
}




// void initializeEncoders() {
//     pinMode(encoderPinA, INPUT);
//     pinMode(encoderPinB, INPUT);

//     attachInterrupt(digitalPinToInterrupt(encoderPinA), []() {
//         bool A = digitalRead(encoderPinA);
//         bool B = digitalRead(encoderPinB);
//         if (A == B) {
//             encoderCount++; // Clockwise
//         } else {
//             encoderCount--; // Counter-clockwise
//         }
//     }, CHANGE);

//     attachInterrupt(digitalPinToInterrupt(encoderPinB), []() {
//         bool A = digitalRead(encoderPinA);
//         bool B = digitalRead(encoderPinB);
//         if (A != B) {
//             encoderCount++; // Clockwise
//         } else {
//             encoderCount--; // Counter-clockwise
//         }
//     }, CHANGE);

//     Serial.println("Encoder pins initialized.");
// }






// void initializeEncoders() {
//     pinMode(encoderPinA, INPUT);
//     pinMode(encoderPinB, INPUT);
//     attachInterrupt(digitalPinToInterrupt(encoderPinA), []() { encoderCount++; }, CHANGE);
//     Serial.println("Encoder pins initialized.");
// }






void stopAllRelays() {
    static bool relaysStopped = false; // Track the relay state
    
    if (!relaysStopped) {
        digitalWrite(materialForwardRelay, LOW);
        digitalWrite(manualShearRelay, LOW);
        Serial.println("All relays stopped."); // Print only when state changes
        relaysStopped = true;
    }
}

// void stopAllRelays() {
//     digitalWrite(materialForwardRelay, LOW);
//     digitalWrite(manualShearRelay, LOW);
//     cuttingMaterial = false;
//     feedingMaterial = false;
//     Serial.println("All relays stopped.");
// }

void updateWheelDiameter(float newDiameter) {
    wheelDiameterInches = newDiameter;
    inchesPerPulse = (3.14159265 * wheelDiameterInches) / pulsesPerRevolution;
    Serial.printf("Wheel diameter updated to %.3f inches\n", wheelDiameterInches);

}

// Function to update the shear delay
void updateShearDelay(float newShearDelay) {
    shearDelay = newShearDelay; // Set the new value
    Serial.printf("Shear delay updated to: %d milliseconds\n", shearDelay);
    // If needed, apply this value to hardware or other logic here
}

void handleManualOverrides() {
    if (manualOverride) {
        digitalWrite(materialForwardRelay, HIGH);
    } else {
        digitalWrite(materialForwardRelay, LOW);
    }

    if (manualShearOverride) {
        digitalWrite(manualShearRelay, HIGH);
    } else {
        digitalWrite(manualShearRelay, LOW);
    }

    Serial.println("Manual Overrides processed.");
    delay(5);
}


void handleCuttingProcess() {
    float adjustedLength = getAdjustedCutDistance(inputLength);
    travelDistanceInches = encoderCount * inchesPerPulse;


    if (travelDistanceInches < adjustedLength) {  // Use corrected length
        digitalWrite(materialForwardRelay, HIGH);
        cuttingMaterial = true;
    } else {
        digitalWrite(materialForwardRelay, LOW);
        Serial.println("Target length reached. Material feed stopped.");
        
        digitalWrite(manualShearRelay, HIGH);
        Serial.println("Shear process started.");
        delay(500);  // Wait for the shear cycle to complete

        digitalWrite(manualShearRelay, LOW);
        Serial.println("Shear process completed. Starting material feed...");
        delay(shearDelay);

        cutCount++;
        sendCutCount();

        if (isReset) {
            inputQuantity = 0;
            cuttingMaterial = false;
            Serial.println("Reset triggered after completing current cut.");
            return;
        }

        inputQuantity--;
        encoderCount = 0;
        travelDistanceInches = 0.0;
        cuttingMaterial = false;

        // Check if cutting is completed
        if (inputQuantity <= 0) {
            // Emit `cutting_completed` event
            StaticJsonDocument<128> doc;
            doc["cutCount"] = cutCount;

            char payload[128];
            serializeJson(doc, payload);

            socket.emit("cutting_completed", payload);

            Serial.println("Cutting process completed. Cutting completed event sent.");
        }
    }
}




// void handleCuttingProcess() {
//     travelDistanceInches = encoderCount * inchesPerPulse;

//     if (travelDistanceInches < inputLength) {
//         digitalWrite(materialForwardRelay, HIGH);
//         cuttingMaterial = true;
//     } else {
//         digitalWrite(materialForwardRelay, LOW);
//         Serial.println("Target length reached. Material feed stopped.");
        
//         digitalWrite(manualShearRelay, HIGH);
//         Serial.println("Shear process started.");
//         delay(500);  // Wait for the shear cycle to complete

//         digitalWrite(manualShearRelay, LOW);
//         Serial.println("Shear process completed. Starting material feed...");
//         delay(shearDelay);

//         cutCount++;
//         sendCutCount();

//         if (isReset) {
//             inputQuantity = 0;
//             cuttingMaterial = false;
//             Serial.println("Reset triggered after completing current cut.");
//             return;
//         }

//         inputQuantity--;
//         encoderCount = 0;
//         travelDistanceInches = 0.0;
//         cuttingMaterial = false;

//         // Check if cutting is completed
//         if (inputQuantity <= 0) {
//             // Emit `cutting_completed` event
//             StaticJsonDocument<128> doc;
//             doc["cutCount"] = cutCount;

//             char payload[128];
//             serializeJson(doc, payload);

//             socket.emit("cutting_completed", payload);

//             Serial.println("Cutting process completed. Cutting completed event sent.");
//         }
//     }
// }

// void handleCuttingProcess() {
//     travelDistanceInches = encoderCount * inchesPerPulse;

//     if (travelDistanceInches < inputLength) {
//         digitalWrite(materialForwardRelay, HIGH);
//         cuttingMaterial = true;
//     } else {
//         digitalWrite(materialForwardRelay, LOW);
//         Serial.println("Target length reached. Material feed stopped.");
        
//         digitalWrite(manualShearRelay, HIGH);
//         Serial.println("Shear process started.");
//         delay(500);  // Wait for the shear cycle to complete

//         digitalWrite(manualShearRelay, LOW);
//         Serial.println("Shear process completed. Starting material feed...");
//         delay(shearDelay);

//         cutCount++;
//         sendCutCount();

//         if (isReset) {
//             inputQuantity = 0;
//             cuttingMaterial = false;
//             Serial.println("Reset triggered after completing current cut.");
//             return;
//         }

//         inputQuantity--;
//         encoderCount = 0;
//         travelDistanceInches = 0.0;
//         cuttingMaterial = false;

//         // Check if cutting is completed
//         if (inputQuantity <= 0) {
//             // Emit `cutting_completed` event
//             StaticJsonDocument<128> doc;
//             doc["cutCount"] = cutCount;
//             doc["cutCycleTime"] = formatCycleTime(); // Helper function to format the cycle time

//             char payload[128];
//             size_t len = serializeJson(doc, payload);

//             socket.emit("cutting_completed", payload);

//             Serial.println("Cutting process completed. Cutting completed event sent.");
//         }
//     }
// }


// void handleCuttingProcess() {
//     travelDistanceInches = encoderCount * inchesPerPulse;

//     if (travelDistanceInches < inputLength) {
//         digitalWrite(materialForwardRelay, HIGH);
//         cuttingMaterial = true;
//     } else {
//         digitalWrite(materialForwardRelay, LOW);
//         Serial.println("Target length reached. Material feed stopped.");
        
//         digitalWrite(manualShearRelay, HIGH);
//         Serial.println("Shear process started.");
//         delay(shearDelay);  // Wait for 6 seconds to complete the shear cycle


//         digitalWrite(manualShearRelay, LOW);
//         Serial.println("Shear Start Switch triggered. Starting material feed...");

//         cutCount++;
//         sendCutCount();

//         if (isReset) {
//             inputQuantity = 0;
//             cuttingMaterial = false;
//             Serial.println("Reset triggered after completing current cut.");
//             return;
//         }

//         inputQuantity--;
//         encoderCount = 0;
//         travelDistanceInches = 0.0;
//         cuttingMaterial = false;
//     }
// }

void handleResetState() {
    if (cuttingMaterial || (inputQuantity > 0 && !isPaused)) {
        Serial.println("Cannot reset while cutting or feeding material. Pause the machine first.");
        isReset = false;
        return;
    }

    Serial.println("Resetting machine...");
    digitalWrite(materialForwardRelay, LOW);
    digitalWrite(manualShearRelay, LOW);
    encoderCount = 0;
    travelDistanceInches = 0.0;
    inputLength = 0;
    inputQuantity = 0;
    cutCount = 0;
    isPaused = false;
    cuttingMaterial = false;
    isReset = false;
    Serial.println("System reset completed. Ready for new parameters.");
}


