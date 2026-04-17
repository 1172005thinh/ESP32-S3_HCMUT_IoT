#include "device_relay.h"
#include "global.h"

// Define independent pins for the 2 new devices (e.g., Relays) for Task 4
// So that it does not conflict with Task 1 (LED_GPIO 48)
#define RELAY1_GPIO 47 // User can reconfigure these later
#define RELAY2_GPIO 46

void device_relay_task(void *pvParameters){
  // AppContext* ctx = (AppContext*)pvParameters;
  
  pinMode(RELAY1_GPIO, OUTPUT);
  pinMode(RELAY2_GPIO, OUTPUT);
  
  bool relay1_state = false;
  bool relay2_state = false;
  
  while(1) {                        
    DeviceCommand cmd;
    // Block up to max delay waiting for a command from WebSocket 
    if (xQueueReceive(xDeviceCommandQueue, &cmd, portMAX_DELAY) == pdPASS) {
        if (cmd.device_id == 1) {
            relay1_state = !relay1_state;
            digitalWrite(RELAY1_GPIO, relay1_state ? HIGH : LOW);
            Serial.printf("Relay 1 toggled to: %d\n", relay1_state);
        } else if (cmd.device_id == 2) {
            relay2_state = !relay2_state;
            digitalWrite(RELAY2_GPIO, relay2_state ? HIGH : LOW);
            Serial.printf("Relay 2 toggled to: %d\n", relay2_state);
        }
    }
  }
}