#include "task_wifi.h"

void startAP()
{
    WiFi.mode(WIFI_AP_STA);

    WiFi.softAP("ESP32_LOCAL", "12345678");

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void startSTA(AppContext* ctx)
{
    xSemaphoreTake(ctx->mutex, portMAX_DELAY);
    String ssid = ctx->WIFI_SSID;
    String pass = ctx->WIFI_PASS;
    xSemaphoreGive(ctx->mutex);

    if (ssid.isEmpty()) return; // ❌ KHÔNG delete task nữa

    WiFi.mode(WIFI_AP_STA); // 🔥 giữ luôn AP

    if (pass.isEmpty())
        WiFi.begin(ssid.c_str());
    else
        WiFi.begin(ssid.c_str(), pass.c_str());

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 50)
    {
        vTaskDelay(200 / portTICK_PERIOD_MS);
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Connected WiFi!");
        xSemaphoreGive(ctx->semInternet);
    }
}

bool Wifi_reconnect(AppContext* ctx)
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    startSTA(ctx);
    return false;
}
