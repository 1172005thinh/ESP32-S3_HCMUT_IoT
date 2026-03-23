#include "coreiot.h"

// ----------- CONFIGURE THESE! -----------
const char* coreIOT_Server = "app.coreiot.io";  // CoreIOT Server URL
const char* coreIOT_Token = "4gkjmsdotg3yfc2jjojv";   // Device Access Token
const int   mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);


void reconnect(AppContext* ctx) {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    xSemaphoreTake(ctx->mutex, portMAX_DELAY);
    String token = ctx->CORE_IOT_TOKEN;
    xSemaphoreGive(ctx->mutex);

    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // ThingsBoard/CoreIoT uses the Access Token as the MQTT username
    if (client.connect(clientId.c_str(), token.c_str(), NULL)) {
        
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* method = doc["method"];
  if (strcmp(method, "setStateLED") == 0) {
    // Check params type (could be boolean, int, or string according to your RPC)
    // Example: {"method": "setValueLED", "params": "ON"}
    const char* params = doc["params"];

    if (strcmp(params, "ON") == 0) {
      Serial.println("Device turned ON.");
      //TODO

    } else {   
      Serial.println("Device turned OFF.");
      //TODO

    }
  } else {
    Serial.print("Unknown method: ");
    Serial.println(method);
  }
}


void setup_coreiot(AppContext* ctx){

  while(1){
    if (xSemaphoreTake(ctx->semInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    Serial.print(".");
  }


  Serial.println(" Connected!");

  xSemaphoreTake(ctx->mutex, portMAX_DELAY);
  String server = ctx->CORE_IOT_SERVER;
  int port = ctx->CORE_IOT_PORT.toInt();
  xSemaphoreGive(ctx->mutex);

  client.setServer(server.c_str(), port);
  client.setCallback(callback);

}

void coreiot_task(void *pvParameters){
    AppContext* ctx = (AppContext*)pvParameters;
    setup_coreiot(ctx);

    while(1){

        if (!client.connected()) {
            reconnect(ctx);
        }
        client.loop();

        xSemaphoreTake(ctx->mutex, portMAX_DELAY);
        float temp = ctx->temperature;
        float humi = ctx->humidity;
        xSemaphoreGive(ctx->mutex);

        // Sample payload, publish to 'v1/devices/me/telemetry'
        String payload = "{\"temperature\":" + String(temp) +  ",\"humidity\":" + String(humi) + "}";
        
        client.publish("v1/devices/me/telemetry", payload.c_str());
        
        Serial.println("Published payload: " + payload);
        vTaskDelay(10000 / portTICK_PERIOD_MS);  // Publish every 10 seconds
    }
}