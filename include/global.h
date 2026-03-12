#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern float glob_temperature;
extern float glob_humidity;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;

// Semaphores for Task 1: Temperature Conditions
extern SemaphoreHandle_t xSemNormalTemp;
extern SemaphoreHandle_t xSemWarningTemp;
extern SemaphoreHandle_t xSemCriticalTemp;

// Semaphores for Task 2: Humidity Conditions
extern SemaphoreHandle_t xSemLowHumi;
extern SemaphoreHandle_t xSemNormalHumi;
extern SemaphoreHandle_t xSemHighHumi;

#endif