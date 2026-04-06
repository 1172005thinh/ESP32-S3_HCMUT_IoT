#include "global.h"
#include <LiquidCrystal_I2C.h>

void lcd_task(void *pvParameters) {
    AppContext* ctx = (AppContext*)pvParameters;
    
    LiquidCrystal_I2C lcd(0x21, 16, 2); // Typical I2C address is 0x27
    lcd.begin();
    lcd.backlight();
    
    lcd.setCursor(0, 0);
    lcd.print("System Ready");

    int current_state = 0; // 0: Normal, 1: Warning, 2: Critical

    while(1) {
        // Check which semaphore is available (don't block indefinitely)
        if (xSemaphoreTake(ctx->semLcdNormal, 0) == pdTRUE) {
            current_state = 0;
        }
        if (xSemaphoreTake(ctx->semLcdWarning, 0) == pdTRUE) {
            current_state = 1;
        }
        if (xSemaphoreTake(ctx->semLcdCritical, 0) == pdTRUE) {
            current_state = 2;
        }

        // Display current state
        lcd.clear();
        lcd.setCursor(0, 0);
        if (current_state == 0) {
            lcd.print("State: NORMAL");
        } else if (current_state == 1) {
            lcd.print("State: WARNING");
        } else {
            lcd.print("State: CRITICAL");
        }

        // Get latest readings safely
        xSemaphoreTake(ctx->mutex, portMAX_DELAY);
        float temp = ctx->temperature;
        float humi = ctx->humidity;
        xSemaphoreGive(ctx->mutex);

        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.print(temp, 1);
        lcd.print(" H:");
        lcd.print(humi, 1);
        
        vTaskDelay(100 / portTICK_PERIOD_MS); // Update screen 0.1s
    }
}