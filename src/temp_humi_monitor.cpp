#include "temp_humi_monitor.h"
#include "task_webserver.h"

DHT20 dht20;

void temp_humi_monitor(void *pvParameters){

    AppContext* ctx = (AppContext*)pvParameters;

    Wire.begin(11, 12);
    // Serial is already started in main
    dht20.begin();

    while (1){
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
            //return;
        }

        //Update shared variables for temperature and humidity via Mutex
        xSemaphoreTake(ctx->mutex, portMAX_DELAY);
        ctx->temperature = temperature;
        ctx->humidity = humidity;
        xSemaphoreGive(ctx->mutex);

        // Feed data to the ML task
        SensorData data = {temperature, humidity};
        xQueueSend(xSensorDataQueue, &data, 0);

        // Broadcast sensor data to Websocket
        StaticJsonDocument<256> doc;
        doc["page"] = "home";
        JsonObject value = doc.createNestedObject("value");
        value["temperature"] = temperature;
        value["humidity"] = humidity;
        
        String jsonStr;
        serializeJson(doc, jsonStr);
        Webserver_sendata(jsonStr);

        // Task 1: Check temperature conditions and give semaphores
        if (temperature < 28.0) {
            xSemaphoreGive(ctx->semNormalTemp);
            xSemaphoreGive(ctx->semLcdNormal);
        } else if (temperature >= 28.0 && temperature <= 35.0) {
            xSemaphoreGive(ctx->semWarningTemp);
            xSemaphoreGive(ctx->semLcdWarning);
        } else {
            xSemaphoreGive(ctx->semCriticalTemp);
            xSemaphoreGive(ctx->semLcdCritical);
        }

        // Task 2: Check humidity conditions and give semaphores
        if (humidity < 40.0) {
            xSemaphoreGive(ctx->semLowHumi);
        } else if (humidity >= 40.0 && humidity <= 60.0) {
            xSemaphoreGive(ctx->semNormalHumi);
        } else {
            xSemaphoreGive(ctx->semHighHumi);
        }

        // Print the results
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}