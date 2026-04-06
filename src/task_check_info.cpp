#include "task_check_info.h"

void Load_info_File(AppContext* ctx)
{
  File file = LittleFS.open("/info.dat", "r");
  if (file)
  {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
    }
    else
    {
      xSemaphoreTake(ctx->mutex, portMAX_DELAY);
      ctx->WIFI_SSID = strdup(doc["WIFI_SSID"]);
      ctx->WIFI_PASS = strdup(doc["WIFI_PASS"]);
      ctx->CORE_IOT_TOKEN = strdup(doc["CORE_IOT_TOKEN"]);
      ctx->CORE_IOT_SERVER = strdup(doc["CORE_IOT_SERVER"]);
      ctx->CORE_IOT_PORT = strdup(doc["CORE_IOT_PORT"]);
      xSemaphoreGive(ctx->mutex);
    }
    file.close();
  }

  // Hardcode LAN Wi-Fi & CoreIOT parameters
  xSemaphoreTake(ctx->mutex, portMAX_DELAY);
  ctx->WIFI_SSID = "HungThinhA56";
  ctx->WIFI_PASS = "";
  ctx->CORE_IOT_SERVER = "app.coreiot.io";
  ctx->CORE_IOT_TOKEN = "";
  ctx->CORE_IOT_PORT = "1883";
  xSemaphoreGive(ctx->mutex);
}

void Delete_info_File()
{
  if (LittleFS.exists("/info.dat"))
  {
    LittleFS.remove("/info.dat");
  }
  ESP.restart();
}

void Save_info_File(String wifi_ssid, String wifi_pass, String CORE_IOT_TOKEN, String CORE_IOT_SERVER, String CORE_IOT_PORT)
{
  Serial.println(wifi_ssid);
  Serial.println(wifi_pass);

  DynamicJsonDocument doc(4096);
  doc["WIFI_SSID"] = wifi_ssid;
  doc["WIFI_PASS"] = wifi_pass;
  doc["CORE_IOT_TOKEN"] = CORE_IOT_TOKEN;
  doc["CORE_IOT_SERVER"] = CORE_IOT_SERVER;
  doc["CORE_IOT_PORT"] = CORE_IOT_PORT;

  File configFile = LittleFS.open("/info.dat", "w");
  if (configFile)
  {
    serializeJson(doc, configFile);
    configFile.close();
  }
  else
  {
    Serial.println("Unable to save the configuration.");
  }
  ESP.restart();
};

bool check_info_File(bool check, AppContext* ctx)
{
  if (!check)
  {
    if (!LittleFS.begin(true))
    {
      Serial.println("❌ Lỗi khởi động LittleFS!");
      return false;
    }
    Load_info_File(ctx);
  }
  
  bool isEmpty = false;
  xSemaphoreTake(ctx->mutex, portMAX_DELAY);
  isEmpty = (ctx->WIFI_SSID.isEmpty() && ctx->WIFI_PASS.isEmpty());
  xSemaphoreGive(ctx->mutex);

  if (!check)
  {
    startAP();
  }

  if (isEmpty)
  {
    return false;
  }
  return true;
}