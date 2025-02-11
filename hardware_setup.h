// #include "SocketIoClient.h"


#ifndef HARDWARE_SETUP_H
#define HARDWARE_SETUP_H

#include <Arduino.h>
#include <SocketIoClient.h>
#include "pins.h"


// extern volatile long encoderCount;

extern bool cuttingMaterial;
extern bool feedingMaterial;

// Encoder-related variables (declare as extern for shared access)

extern volatile long encoderCount;
extern volatile unsigned long lastInterruptTimeA;
extern volatile unsigned long lastInterruptTimeB;
extern const unsigned long debounceDelay;

// Function prototypes
void initializeEncoders();
void handleEncoderA();
void handleEncoderB();


// extern SocketIoClient socket;

void initializeRelays();
void initializeLimitSwitches();
void initializeEncoders();
void stopAllRelays();
void handleManualOverrides();
void handleCuttingProcess();
void handleResetState();
void updateWheelDiameter(float newDiameter); // Declaration
void updateShearDelay(float newShearDelay);



#endif

