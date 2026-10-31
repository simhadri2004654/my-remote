#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "app_config.h"
#include "storage.h"
#include "qr.h"
#include "web_server.h"

namespace
{
    bool storageOK = false;
    bool serverStarted = false;
    bool qrReady = false;
    String lastConnectedIp;
    unsigned long lastWifiRetry = 0;
    unsigned long lastQrRetry = 0;

    bool connectWiFi()
    {
        Serial.println();
        Serial.print(F("Connecting to Wi-Fi: "));
        Serial.println(WIFI_SSID);

        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.hostname("AutomaticQRRotation");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // DHCP: no static IP config.

        const unsigned long startedAt = millis();
        while (WiFi.status() != WL_CONNECTED &&
               millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS)
        {
            delay(300);
            Serial.print('.');
            yield();
        }
        Serial.println();

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println(F("Wi-Fi connection not available yet."));
            return false;
        }

        Serial.print(F("Wi-Fi connected. DHCP IP: "));
        Serial.println(WiFi.localIP());
        return true;
    }

    void startNetworkServices()
    {
        webServerBegin();
        serverStarted = true;

        qrReady = qrGenerateAndDisplayNext();
        if (!qrReady)
        {
            Serial.println(F("QR display failed. Device will retry with a new ID."));
            lastQrRetry = millis();
        }
        else
        {
            lastConnectedIp = WiFi.localIP().toString();
        }
    }

    void fatalStop(const __FlashStringHelper *message)
    {
        Serial.println();
        Serial.println(F("FATAL STOP: counter safety cannot be guaranteed."));
        Serial.println(message);
        Serial.println(F("Do not format LittleFS or reset the QR counter automatically."));
        while (true)
        {
            delay(1000);
            yield();
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(300);

    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("AUTOMATIC QR ROTATION SYSTEM"));
    Serial.println(F("ESP8266MOD + Waveshare 3.5-inch E-Paper"));
    Serial.println(F("========================================"));

    // Initialize counter storage first. Never start with ID 1 if the ledger
    // is damaged, because that could accidentally repeat previously issued IDs.
    storageOK = storageBegin();
    if (!storageOK)
        fatalStop(F("LittleFS QR counter ledger could not be initialized."));

    qrBegin();

    if (connectWiFi())
    {
        startNetworkServices();
    }
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        if (serverStarted)
        {
            webServerStop();
            serverStarted = false;
            qrReady = false;
            Serial.println(F("Wi-Fi disconnected; waiting to reconnect."));
        }

        if (millis() - lastWifiRetry >= WIFI_RETRY_INTERVAL_MS)
        {
            lastWifiRetry = millis();
            if (connectWiFi())
                startNetworkServices();
        }

        delay(2);
        yield();
        return;
    }

    if (!serverStarted)
    {
        startNetworkServices();
        return;
    }

    webServerHandleClient();

    // A successful /scan request invalidates the old ID and sets this flag.
    // The HTTP customer page has already been sent before the E-Paper refresh.
    if (webServerTakeQrRefreshRequest())
    {
        Serial.println(F("Successful scan: preparing the next unique QR..."));

        qrReady = qrGenerateAndDisplayNext();
        if (!qrReady)
        {
            // Keep the current token invalid and retry later. A failed ID is
            // intentionally skipped rather than reused.
            Serial.println(F("QR update failed; will retry with a fresh ID."));
            lastQrRetry = millis();
        }
    }

    // If the DHCP lease changes, the old URL will no longer reach this unit.
    const String currentIp = WiFi.localIP().toString();
    if (qrReady && currentIp != lastConnectedIp)
    {
        Serial.println(F("DHCP IP changed. Invalidating old QR and displaying a new one."));
        webServerInvalidateActiveToken();
        qrReady = false;
        if (qrGenerateAndDisplayNext())
        {
            qrReady = true;
            lastConnectedIp = currentIp;
        }
        else
        {
            lastQrRetry = millis();
        }
    }

    // Recover from a transient display/QR failure without reusing an ID.
    if (!qrReady && millis() - lastQrRetry >= 5000UL)
    {
        lastQrRetry = millis();
        qrReady = qrGenerateAndDisplayNext();
        if (qrReady)
            lastConnectedIp = WiFi.localIP().toString();
    }

    delay(2);
    yield();
}
