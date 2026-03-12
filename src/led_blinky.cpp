#include "led_blinky.h"
#include "global.h"

void led_blinky(void *pvParameters){
  pinMode(LED_GPIO, OUTPUT);
  int current_state = 0; // 0: Normal, 1: Warning, 2: Critical
  
  while(1) {                        
    // Check which semaphore is available (don't block indefinitely, just check with 0 ticks)
    if (xSemaphoreTake(xSemNormalTemp, 0) == pdTRUE) {
        current_state = 0;
    }
    if (xSemaphoreTake(xSemWarningTemp, 0) == pdTRUE) {
        current_state = 1;
    }
    if (xSemaphoreTake(xSemCriticalTemp, 0) == pdTRUE) {
        current_state = 2;
    }

    if (current_state == 0) {
        // Normal: Blink 1s
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    } 
    else if (current_state == 1) {
        // Warning: Blink fast 200ms
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    } 
    else {
        // Critical: Solid ON
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Just a short delay to yield task
    }
  }
}