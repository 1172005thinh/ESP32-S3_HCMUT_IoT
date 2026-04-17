#include "task_wifi.h"

void startAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(String(SSID_AP), String(PASS_AP));
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

bool startSTA(AppContext* ctx)
{
    xSemaphoreTake(ctx->mutex, portMAX_DELAY);
    String ssid = ctx->WIFI_SSID;
    String pass = ctx->WIFI_PASS;
    xSemaphoreGive(ctx->mutex);

    if (ssid.isEmpty())
    {
        return false;
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

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 120)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        xSemaphoreGive(ctx->semInternet);
        return true;
    }
    return false;
}

bool Wifi_reconnect(AppContext* ctx)
{
    static bool isFallbackAP = false;
    
    if (isFallbackAP) {
        // We failed previously and reverted to AP. 
        // Stay in AP mode until the user reconfigures and the board reboots!
        return false;
    }

    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true; // Already connected
    }
    
    // Attempt STA connection
    if (startSTA(ctx)) {
        return true;
    }

    // Connection failed for X = 120 attempts, revert to AP mode
    Serial.println("===============================");
    Serial.println("[SYS] Failed to connect to STA. Reverting back to AP mode.");
    Serial.println("===============================");
    WiFi.disconnect(true); // Disconnect existing STA session before mode switch
    vTaskDelay(100 / portTICK_PERIOD_MS);
    startAP();
    isFallbackAP = true;
    
    // Return false to let caller know
    return false;
}
