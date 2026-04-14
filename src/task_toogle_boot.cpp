#include "task_toogle_boot.h"

#define BOOT 0

void Task_Toogle_BOOT(void *pvParameters)
{
    pinMode(BOOT, INPUT_PULLUP);
    unsigned long buttonPressStartTime = 0;
    
    Serial.println("[SYS] BOOT button monitor task successfully launched on GPIO 0");

    while (true)
    {
        if (digitalRead(BOOT) == LOW)
        {
            if (buttonPressStartTime == 0)
            {
                Serial.println("[SYS] BOOT button pressed, holding...");
                buttonPressStartTime = millis();
            }
            else if (millis() - buttonPressStartTime > 5000) // Increase to 5 seconds to be safe
            {
                Serial.println("[WARN] BOOT button held >5s! Deleting credentials and resetting...");
                Delete_info_File();
                vTaskDelete(NULL); // Fallback in case restart fails
            }
        }
        else
        {
            if (buttonPressStartTime != 0) {
                Serial.println("[SYS] BOOT button released before 5s.");
            }
            buttonPressStartTime = 0;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}