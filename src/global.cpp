#include "global.h"
float glob_temperature = 0;
float glob_humidity = 0;

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid = "abcde";
String wifi_password = "123456789";
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

// Semaphores for Task 1: Temperature Conditions
SemaphoreHandle_t xSemNormalTemp = NULL;
SemaphoreHandle_t xSemWarningTemp = NULL;
SemaphoreHandle_t xSemCriticalTemp = NULL;

// Semaphores for Task 2: Humidity Conditions
SemaphoreHandle_t xSemLowHumi = NULL;
SemaphoreHandle_t xSemNormalHumi = NULL;
SemaphoreHandle_t xSemHighHumi = NULL;