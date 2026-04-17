#include "coreiot.h"

WiFiClient espClient;
PubSubClient client(espClient);
static String mqttServerHost;
static uint16_t mqttServerPort = 1883;

static const char* wifiStatusToText(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    default: return "WL_UNKNOWN";
  }
}

static const char* mqttStateToText(int state) {
  switch (state) {
    case -4: return "MQTT_CONNECTION_TIMEOUT";
    case -3: return "MQTT_CONNECTION_LOST";
    case -2: return "MQTT_CONNECT_FAILED";
    case -1: return "MQTT_DISCONNECTED";
    case 0: return "MQTT_CONNECTED";
    case 1: return "MQTT_CONNECT_BAD_PROTOCOL";
    case 2: return "MQTT_CONNECT_BAD_CLIENT_ID";
    case 3: return "MQTT_CONNECT_UNAVAILABLE";
    case 4: return "MQTT_CONNECT_BAD_CREDENTIALS";
    case 5: return "MQTT_CONNECT_UNAUTHORIZED";
    default: return "MQTT_STATE_UNKNOWN";
  }
}


void reconnect(AppContext* ctx) {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.printf("[SYS] Attempting MQTT connection to %s:%u...\n", mqttServerHost.c_str(), mqttServerPort);
    
    xSemaphoreTake(ctx->mutex, portMAX_DELAY);
    String token = ctx->CORE_IOT_TOKEN;
    xSemaphoreGive(ctx->mutex);

    if (token.isEmpty()) {
      Serial.println("[ERR] MQTT token is empty, cannot connect");
      delay(5000);
      continue;
    }

    wl_status_t wifiStatus = WiFi.status();
    Serial.printf("[INF] WiFi status: %d (%s), local IP: %s\n", (int)wifiStatus, wifiStatusToText(wifiStatus), WiFi.localIP().toString().c_str());
    Serial.printf("[INF] Gateway: %s, DNS: %s\n", WiFi.gatewayIP().toString().c_str(), WiFi.dnsIP().toString().c_str());

    IPAddress brokerIP;
    int dnsResult = WiFi.hostByName(mqttServerHost.c_str(), brokerIP);
    if (dnsResult == 1) {
      Serial.printf("[INF] DNS OK: %s -> %s\n", mqttServerHost.c_str(), brokerIP.toString().c_str());

      WiFiClient probeClient;
      bool tcpOk = probeClient.connect(brokerIP, mqttServerPort);
      if (tcpOk) {
        Serial.printf("[INF] TCP OK: %s:%u reachable\n", brokerIP.toString().c_str(), mqttServerPort);
        probeClient.stop();
      } else {
        Serial.printf("[ERR] TCP FAILED: cannot reach %s:%u\n", brokerIP.toString().c_str(), mqttServerPort);

        WiFiClient probe443;
        bool tcp443Ok = probe443.connect(brokerIP, 443);
        Serial.printf("[INF] Probe 443: %s\n", tcp443Ok ? "reachable" : "unreachable");
        if (tcp443Ok) {
          probe443.stop();
        }

        WiFiClient probe80;
        bool tcp80Ok = probe80.connect(brokerIP, 80);
        Serial.printf("[INF] Probe 80: %s\n", tcp80Ok ? "reachable" : "unreachable");
        if (tcp80Ok) {
          probe80.stop();
        }

        WiFiClient probe8883;
        bool tcp8883Ok = probe8883.connect(brokerIP, 8883);
        Serial.printf("[INF] Probe 8883: %s\n", tcp8883Ok ? "reachable" : "unreachable");
        if (tcp8883Ok) {
          probe8883.stop();
        }
      }
    } else {
      Serial.printf("[ERR] DNS FAILED for %s, hostByName rc=%d\n", mqttServerHost.c_str(), dnsResult);
    }

    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    Serial.printf("[INF] MQTT clientId=%s, tokenLen=%u\n", clientId.c_str(), (unsigned int)token.length());

    // ThingsBoard/CoreIoT uses the Access Token as the MQTT username
    if (client.connect(clientId.c_str(), token.c_str(), NULL)) {
        
      Serial.println("=================================");
      Serial.println("[SUCCESS] Connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("[SUCCESS] Subscribed to v1/devices/me/rpc/request/+");
      Serial.println("=================================");

    } else {
      int state = client.state();
      Serial.printf("failed, rc=%d (%s)\n", state, mqttStateToText(state));
      Serial.printf("[INF] MQTT endpoint was %s:%u\n", mqttServerHost.c_str(), mqttServerPort);
      Serial.println("[SYS] Retry in 5 seconds");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[MSG] Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("[MSG] Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("[ERR] deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* method = doc["method"];
  if (strcmp(method, "setStateLED") == 0) {
    // Check params type (could be boolean, int, or string according to your RPC)
    // Example: {"method": "setValueLED", "params": "ON"}
    const char* params = doc["params"];

    if (strcmp(params, "ON") == 0) {
      Serial.println("[MSG] Device turned ON.");
      //TODO

    } else {   
      Serial.println("[MSG] Device turned OFF.");
      //TODO

    }
  } else {
    Serial.print("[MSG] Unknown method: ");
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


  Serial.println("[SUCCESS] Connected!");

  xSemaphoreTake(ctx->mutex, portMAX_DELAY);
  mqttServerHost = ctx->CORE_IOT_SERVER;
  mqttServerPort = (uint16_t)ctx->CORE_IOT_PORT.toInt();
  xSemaphoreGive(ctx->mutex);

  if (mqttServerHost.isEmpty()) {
    Serial.println("[INF] CoreIOT server is empty, skip MQTT setup");
    return;
  }

  if (mqttServerPort == 0) {
    mqttServerPort = 1883;
  }

  // Keep host in static storage because PubSubClient stores this pointer.
  client.setServer(mqttServerHost.c_str(), mqttServerPort);
  client.setCallback(callback);

}

void coreiot_task(void *pvParameters){
    AppContext* ctx = (AppContext*)pvParameters;
    setup_coreiot(ctx);

    bool anomalyDetected = false;

    while(1){
        // Don't attempt to connect to CoreIOT if we've fallen back to AP mode
        if (WiFi.getMode() == WIFI_AP) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        if (!client.connected()) {
            reconnect(ctx);
        }
        client.loop();

        // Check anomaly queue without blocking
        xQueueReceive(xAnomalyQueueIOT, &anomalyDetected, 0);

        xSemaphoreTake(ctx->mutex, portMAX_DELAY);
        float temp = ctx->temperature;
        float humi = ctx->humidity;
        xSemaphoreGive(ctx->mutex);

        // Sample payload, publish to 'v1/devices/me/telemetry'
        String payload = "{\"temperature\":" + String(temp) +  ",\"humidity\":" + String(humi) + ",\"anomaly\":" + (anomalyDetected ? "true" : "false") + "}";
        
        client.publish("v1/devices/me/telemetry", payload.c_str());
        
        Serial.println("[MSG] Published payload: " + payload);
        vTaskDelay(10000 / portTICK_PERIOD_MS);  // Publish every 10 seconds
    }
}