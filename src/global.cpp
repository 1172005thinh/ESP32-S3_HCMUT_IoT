#include "global.h"

// All global variables have been removed to comply with Task 3.
// Data is now passed via the AppContext struct and protected by a Mutex semaphore.

QueueHandle_t xSensorDataQueue;
QueueHandle_t xAnomalyQueueLCD;
QueueHandle_t xAnomalyQueueIOT;
QueueHandle_t xDeviceCommandQueue;