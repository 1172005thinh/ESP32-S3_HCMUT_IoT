#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
// #include "mainserver.h"
// #include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"
#include "task_lcd.h"

void main_manager_task(void *pvParameters) {
    AppContext* ctx = (AppContext*)pvParameters;
    while(1) {
        if (check_info_File(1, ctx))
        {
            if (!Wifi_reconnect(ctx))
            {
                Webserver_stop();
            }
        }
        Webserver_reconnect();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void setup()
{
  Serial.begin(115200);
  startAP(); 
  AppContext* ctx = new AppContext();
  ctx->mutex = xSemaphoreCreateMutex();
  ctx->semInternet = xSemaphoreCreateBinary();
  ctx->semNormalTemp = xSemaphoreCreateBinary();
  ctx->semWarningTemp = xSemaphoreCreateBinary();
  ctx->semCriticalTemp = xSemaphoreCreateBinary();
  ctx->semLowHumi = xSemaphoreCreateBinary();
  ctx->semNormalHumi = xSemaphoreCreateBinary();
  ctx->semHighHumi = xSemaphoreCreateBinary();
  ctx->semLcdNormal = xSemaphoreCreateBinary();
  ctx->semLcdWarning = xSemaphoreCreateBinary();
  ctx->semLcdCritical = xSemaphoreCreateBinary();

  check_info_File(0, ctx);

  xTaskCreate(led_blinky, "Task LED Blink", 2048, ctx, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, ctx, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, ctx, 2, NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,ctx  ,2 , NULL);
  xTaskCreate(lcd_task, "LCD Task", 2048, ctx, 2, NULL);
  xTaskCreate(main_manager_task, "Main Manager", 4096, ctx, 2, NULL);
}

void loop()
{
  vTaskDelete(NULL);
}