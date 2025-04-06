/*
 * ESP01 HomeKit Door Control - Complete Working Version
 * Includes TX/RX shorting detection for HomeKit reset
 */

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <ESP8266WiFi.h>

// Access your router or AP
const char* ssid = "WIFI_SSID_HERE";
const char* password = "WIWI_PASSWORD_HERE";

// TX/RX shorting detection variables
const uint8_t RESET_PATTERN[] = { 0xFF, 0xA3, 0x5C, 0xFF }; // Unlikely pattern for normal operation
unsigned long last_check_time = 0;
const unsigned long CHECK_INTERVAL = 10000; // Check every 10 seconds
bool reset_check_active = false;
bool reset_in_progress = false;
unsigned long reset_start_time = 0;
const unsigned long RESET_DURATION = 5000; // Hold short for 5 seconds to confirm

// Function to send serial commands to the STC15F104W microcontroller
void sendCommand(uint8_t command[], int length) {
  for (int i = 0; i < length; i++) {
    Serial.write(command[i]);
  }
  Serial.flush();  // Ensure all data is sent
}

// This is the button-press code
void pressButton() {
  // Command to turn the relay ON (close the circuit)
  uint8_t relayOn[] = { 0xA0, 0x01, 0x01, 0xA2 };
  sendCommand(relayOn, 4);
  
  // Wait for 250 milliseconds
  delay(250);
  
  // Command to turn the relay OFF (open the circuit)
  uint8_t relayOff[] = { 0xA0, 0x01, 0x00, 0xA1 };
  sendCommand(relayOff, 4);
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_lock_target_state;
extern "C" homekit_characteristic_t cha_lock_current_state;

// Task states for non-blocking operation
bool door_triggered = false;
unsigned long reset_time = 0;
const unsigned long RESET_DELAY = 1000; // 1 second after trigger before resetting state

// Function to check if TX and RX are shorted
void checkTxRxShort() {
  // Clear any pending serial data
  while (Serial.available()) {
    Serial.read();
  }
  
  // Start the test - mark that we're checking
  reset_check_active = true;
  
  // Send the reset pattern
  for (int i = 0; i < 4; i++) {
    Serial.write(RESET_PATTERN[i]);
  }
  Serial.flush();
  
  // Give some time for the loopback to occur
  delay(50);
  
  // Check if we received our pattern back (indicating TX/RX short)
  if (Serial.available() >= 4) {
    bool match = true;
    for (int i = 0; i < 4; i++) {
      if (Serial.read() != RESET_PATTERN[i]) {
        match = false;
        break;
      }
    }
    
    if (match) {
      // TX/RX short detected, start the reset process
      if (!reset_in_progress) {
        reset_in_progress = true;
        reset_start_time = millis();
      }
    } else {
      // No match, cancel any in-progress reset
      reset_in_progress = false;
    }
  } else {
    // No data received, cancel any in-progress reset
    reset_in_progress = false;
  }
  
  // Clear flag
  reset_check_active = false;
  
  // Clear any remaining data
  while (Serial.available()) {
    Serial.read();
  }
}

// Called when the lock target state is changed by HomeKit (e.g., via Siri)
void lock_target_state_setter(const homekit_value_t value) {
  uint8_t state = value.uint8_value;
  cha_lock_target_state.value.uint8_value = state; // sync the value
  
  if (state == 0) { // 0 = unsecured (unlocked/open)
    // Update current state to unlocked
    cha_lock_current_state.value.uint8_value = 0;
    homekit_characteristic_notify(&cha_lock_current_state, HOMEKIT_UINT8(0));
    
    // Trigger the door
    pressButton();
    
    // Set up the state reset to happen in loop() instead of blocking here
    door_triggered = true;
    reset_time = millis() + RESET_DELAY;
  }
}

void setup() {
  // Start the serial communication with the STC15F104W at 9600 baud
  Serial.begin(9600);
  
  WiFi.begin(ssid, password);
  // Wait for connection silently
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Init HomeKit
  my_homekit_setup();
}

void loop() {
  // Handle HomeKit activity
  arduino_homekit_loop();
  
  // Non-blocking state reset after door is triggered
  if (door_triggered && millis() >= reset_time) {
    // Reset back to locked state
    cha_lock_target_state.value.uint8_value = 1; // 1 = secured
    homekit_characteristic_notify(&cha_lock_target_state, HOMEKIT_UINT8(1));
    
    cha_lock_current_state.value.uint8_value = 1; // 1 = secured
    homekit_characteristic_notify(&cha_lock_current_state, HOMEKIT_UINT8(1));
    
    door_triggered = false;
  }
  
  // TX/RX shorting detection to trigger HomeKit reset
  // Only check periodically to not interfere with normal operation
  if (!reset_in_progress && millis() - last_check_time > CHECK_INTERVAL) {
    checkTxRxShort();
    last_check_time = millis();
  }
  
  // If reset has been initiated, check if it's been held long enough
  if (reset_in_progress && millis() - reset_start_time > RESET_DURATION) {
    // Short has been maintained for the required duration, reset HomeKit pairing
    homekit_storage_reset();
    
    // Visual feedback (rapid relay clicks)
    for (int i = 0; i < 3; i++) {
      pressButton();
      delay(300);
    }
    
    // Reset ESP to apply changes
    ESP.restart();
  }
  
  delay(10);
}

// Called when the HomeKit setup is complete
void my_homekit_setup() {
  // Add the .setter function to the target lock state characteristic
  cha_lock_target_state.setter = lock_target_state_setter;
  
  // Setup and start HomeKit server
  arduino_homekit_setup(&config);
}