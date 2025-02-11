#include "utility.h"
#include "config.h"


void sendTravelDistance() {
    StaticJsonDocument<128> doc;
    doc["travelDistance"] = encoderCount * inchesPerPulse;

    String output;
    serializeJson(doc, output);
    socket.emit("travel_distance", output.c_str());
    Serial.println(output);
}

void sendCutCount() {
    StaticJsonDocument<128> doc;
    doc["cutCount"] = cutCount;

    String output;
    serializeJson(doc, output);
    socket.emit("cut_status", output.c_str());
    Serial.println(output);
}

// Function to format elapsed time as HH:MM:SS
// String formatCycleTime() {
//     unsigned long totalSeconds = millis() / 1000;
//     unsigned long hours = totalSeconds / 3600;
//     unsigned long minutes = (totalSeconds % 3600) / 60;
//     unsigned long seconds = totalSeconds % 60;

//     char buffer[16];
//     snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
//     return String(buffer);
// }