#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Struct to hold all system data, replacing global variables
struct AppContext {
    // Sensor Data
    float temperature;
    float humidity;

    // Config Data
    String WIFI_SSID;
    String WIFI_PASS;
    String CORE_IOT_TOKEN;
    String CORE_IOT_SERVER;
    String CORE_IOT_PORT;

    // Mutex for protecting read/write of this struct
    SemaphoreHandle_t mutex;

    // Semaphores for task synchronization
    SemaphoreHandle_t semInternet;
    SemaphoreHandle_t semNormalTemp;
    SemaphoreHandle_t semWarningTemp;
    SemaphoreHandle_t semCriticalTemp;
    SemaphoreHandle_t semLowHumi;
    SemaphoreHandle_t semNormalHumi;
    SemaphoreHandle_t semHighHumi;
    
    // Semaphores for Task 3: LCD States
    SemaphoreHandle_t semLcdNormal;
    SemaphoreHandle_t semLcdWarning;
    SemaphoreHandle_t semLcdCritical;
};

// Struct to hold sensor data for ML queue
struct SensorData {
    float temperature;
    float humidity;
};

extern QueueHandle_t xSensorDataQueue;
extern QueueHandle_t xAnomalyQueueLCD;
extern QueueHandle_t xAnomalyQueueIOT;

#endif