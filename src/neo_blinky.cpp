#include "neo_blinky.h"
#include "global.h"

void neo_blinky(void *pvParameters){
    AppContext* ctx = (AppContext*)pvParameters;

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();

    int current_state = 1; // 0: Low, 1: Normal, 2: High

    while(1) {                          
        // Check which semaphore is available (don't block indefinitely)
        if (xSemaphoreTake(ctx->semLowHumi, 0) == pdTRUE) {
            current_state = 0;
        }
        if (xSemaphoreTake(ctx->semNormalHumi, 0) == pdTRUE) {
            current_state = 1;
        }
        if (xSemaphoreTake(ctx->semHighHumi, 0) == pdTRUE) {
            current_state = 2;
        }

        // Set color based on state
        if (current_state == 0) {
            // Low Humidity: Blue
            strip.setPixelColor(0, strip.Color(0, 0, 255));
        } else if (current_state == 1) {
            // Normal Humidity: Green
            strip.setPixelColor(0, strip.Color(0, 255, 0));
        } else {
            // High Humidity: Red
            strip.setPixelColor(0, strip.Color(255, 0, 0));
        }

        strip.show(); // Update the strip

        // Delay to yield the task
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}