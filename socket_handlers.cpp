#include "esp32-hal-gpio.h"
#include "socket_handlers.h"
#include "config.h"

bool socketConnected = false; // Flag to track socket connection state

void configureSocketIOHandlers() {
    socket.on("connect", [](const char* payload, size_t length) {
        socketConnected = true; // Set connection flag
        Serial.println("Connected to Socket.IO server");
    });

    socket.on("disconnect", [](const char* payload, size_t length) {
        socketConnected = false; // Clear connection flag
        Serial.println("Disconnected from Socket.IO server");
    });

    socket.on("confirm_reset", [](const char* payload, size_t length) {
        if (!socketConnected) return;
        Serial.println("Confirm reset command received.");
        isReset = true;
    });

    socket.on("material_forward_control", [](const char* payload, size_t length) {
        if (!socketConnected) return;
        Serial.printf("Material Forward Control Command: %s\n", payload);

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error) {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
            return;
        }

        if (doc.containsKey("materialForward")) {
            String command = doc["materialForward"].as<String>();
            if (command == "ON") {
                manualOverride = true;
                digitalWrite(materialForwardRelay, HIGH);
                Serial.println("Material Forward Relay ON (Manual Override).");
            } else if (command == "OFF") {
                manualOverride = false;
                digitalWrite(materialForwardRelay, LOW);
                Serial.println("Material Forward Relay OFF (Manual Override).");
                delay(50);
            } else {
                Serial.println("Invalid Material Forward Command.");
            }
        } else {
            Serial.println("Invalid JSON for material_forward_control.");
        }
    });

    socket.on("manual_shear_control", [](const char* payload, size_t length) {
        if (!socketConnected) return;

        Serial.printf("Manual Shear Control Command: %s\n", payload);
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error) {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
            return;
        }

        if (doc.containsKey("manualShear")) {
            String command = doc["manualShear"].as<String>();
            if (command == "ON") {
                manualShearOverride = true;
                digitalWrite(manualShearRelay, HIGH);
                Serial.println("Manual Shear Relay ON (Manual Override).");
            } else if (command == "OFF") {
                manualShearOverride = false;
                digitalWrite(manualShearRelay, LOW);
                Serial.println("Manual Shear Relay OFF (Manual Override).");
            } else {
                Serial.println("Invalid Manual Shear Command.");
            }
        } else {
            Serial.println("Invalid JSON for manual_shear_control.");
        }
    });

      socket.on("motor_command", [](const char* payload, size_t length) {
      Serial.printf("Received motor_command: %.*s\n", length, payload); // Debug: Print raw payload

      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, payload, length); // Pass `length` for safer parsing

      if (error) {
          Serial.print("JSON parsing failed: ");
          Serial.println(error.c_str());
          return; // Exit if deserialization fails
      }

      if (!doc.containsKey("motor")) {
          Serial.println("Invalid JSON: Missing 'motor' key.");
          return; // Exit if required key is missing
      }

      String motorCommand = doc["motor"].as<String>();
      Serial.printf("Motor Command: %s\n", motorCommand.c_str()); // Debug: Log parsed command

      if (motorCommand == "PAUSE") {
          if (!isPaused) {
              isPaused = true;
              digitalWrite(materialForwardRelay, LOW);
              digitalWrite(manualShearRelay, LOW);
              Serial.println("Motor paused via socket command.");
          } else {
              Serial.println("Motor is already paused.");
          }
      } else if (motorCommand == "RESUME") {
          if (isPaused && !isEStopActive) {
              isPaused = false;
              Serial.println("Motor resumed via socket command.");
          } else if (isEStopActive) {
              Serial.println("Cannot resume. E-Stop is active.");
          } else {
              Serial.println("Resume not required. Machine is already running.");
          }
      } else {
          Serial.printf("Invalid motor command received: %s\n", motorCommand.c_str());
      }
  });

  socket.on("cut_status", [](const char* payload, size_t length) {
    if (!socketConnected) return; // Ensure the socket is connected

    Serial.printf("Cut status payload received: %.*s\n", length, payload);

    // Parse the JSON payload
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Check for required keys
    if (doc.containsKey("inputQuantity")) {
        int inputQuantity = doc["inputQuantity"];
        Serial.printf("Remaining quantity: %d\n", inputQuantity);

        if (inputQuantity == 0) {
            // Emit `cutting_completed` event back to the server
            StaticJsonDocument<128> response;
            response["cutCount"] = doc["cutCount"] | 0; // Fallback to 0 if not provided

            char responsePayload[128];
            serializeJson(response, responsePayload);
            socket.emit("cutting_completed", responsePayload);

            Serial.println("Cutting process completed. Event emitted.");
        }
    } else {
        Serial.println("Invalid JSON: Missing 'inputQuantity' key.");
    }
});


      // Handle Cut Parameters
    socket.on("set_cut_parameters", [](const char* payload, size_t length) {
      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error && doc.containsKey("cutLength") && doc.containsKey("cutQuantity")) {
          inputLength = doc["cutLength"];
          inputQuantity = doc["cutQuantity"];
          Serial.printf("Cut parameters received - Length: %.3f, Quantity: %d\n", inputLength, inputQuantity);

          // Clear reset and pause states before starting
          if (isReset) {
              Serial.println("Clearing reset state.");
              isReset = false;
          }

          if (isPaused) {
              Serial.println("Clearing paused state.");
              isPaused = false;
          }

          if (inputLength > 0 && inputQuantity > 0) {
              encoderCount = 0;
              travelDistanceInches = 0.0;
              digitalWrite(materialForwardRelay, HIGH); // Start material feed
              Serial.println("Motor started for cutting process.");
          } else {
              Serial.println("Invalid parameters. Cannot start the machine.");
          }
      } else {
          Serial.println("Invalid cut parameters payload.");
      }
    });

    socket.on("e_stop", [](const char* payload, size_t length) {
        if (!socketConnected) return;

        isEStopActive = true;
        isResumeRequired = true;
        isPaused = true;
        digitalWrite(materialForwardRelay, LOW);
        digitalWrite(manualShearRelay, LOW);
        // stopAllRelays();
        Serial.println("E-Stop activated: Relays OFF. Resume required.");
    });

    // Handle Reset E-Stop
    socket.on("reset_e_stop", [](const char* payload, size_t length) {
        isEStopActive = false;
        isResumeRequired = false;
        Serial.println("E-Stop reset: Ready for resume.");
    });

    // Handle Reset command
    socket.on("reset", [](const char* payload, size_t length) {
        isReset = true;
        Serial.println("Reset command received. Waiting for current cut to finish...");
    });

socket.on("update_calibration", [](const char* payload, size_t length) {
    Serial.printf("Calibration Update Command: %s\n", payload);

    StaticJsonDocument<256> doc; // Adjust size to accommodate your payload
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Check if "calibration" key exists
    if (doc.containsKey("calibration")) {
        JsonObject calibration = doc["calibration"];

        // Validate and update wheel diameter
        if (calibration.containsKey("wheelDiameter")) {
            float newDiameter = calibration["wheelDiameter"].as<float>();

            if (newDiameter > 0) {
                updateWheelDiameter(newDiameter);
                Serial.printf("Wheel diameter updated to: %.3f inches\n", newDiameter);
            } else {
                Serial.println("Invalid wheel diameter value.");
            }
        } else {
            Serial.println("No 'wheelDiameter' key found in 'calibration'.");
        }

        // Validate and update shear delay
        if (calibration.containsKey("shearDelay")) {
            float newShearDelay = calibration["shearDelay"].as<float>();

            if (newShearDelay > 0) {
                updateShearDelay(newShearDelay);
                Serial.printf("Shear delay updated to: %.3f milliseconds\n", newShearDelay);
            } else {
                Serial.println("Invalid shear delay value.");
            }
        } else {
            Serial.println("No 'shearDelay' key found in 'calibration'.");
        }
    } else {
        Serial.println("No 'calibration' key found in the payload.");
    }
});


    Serial.println("Socket.IO event handlers configured.");
}

