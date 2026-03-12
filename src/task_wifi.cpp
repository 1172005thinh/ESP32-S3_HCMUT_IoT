#include "task_wifi.h"

void startAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(String(SSID_AP), String(PASS_AP));
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void startSTA(AppContext* ctx)
{
    xSemaphoreTake(ctx->mutex, portMAX_DELAY);
    String ssid = ctx->WIFI_SSID;
    String pass = ctx->WIFI_PASS;
    xSemaphoreGive(ctx->mutex);

    if (ssid.isEmpty())
    {
        vTaskDelete(NULL);
    }

    WiFi.mode(WIFI_STA);

    if (pass.isEmpty())
    {
        WiFi.begin(ssid.c_str());
    }
    else
    {
        WiFi.begin(ssid.c_str(), pass.c_str());
    }

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    //Give a semaphore here
    xSemaphoreGive(ctx->semInternet);
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
