#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>

#include "app_config.h"
#include "product_store.h"
#include "qr.h"
#include "web_server.h"

static unsigned long lastReconnectAttempt = 0;
static unsigned long lastQrIpCheck = 0;

static bool ConnectToWiFi()
{
    Serial.println();
    Serial.println(F("Connecting to Wi-Fi using DHCP..."));

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);
    WiFi.hostname("LakshivPriceSystem");

    // Deliberately do not call WiFi.config(): router DHCP supplies the IP.
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const unsigned long startedAt = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startedAt < 30000UL) {
        delay(500);
        Serial.print('.');
        yield();
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("Wi-Fi connected."));
        Serial.print(F("Router-assigned IP: "));
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println(F("Wi-Fi not connected yet. The loop will retry."));
    return false;
}

void setup()
{
    Serial.begin(115200);
    delay(300);

    Serial.println();
    Serial.println(F("=========================================="));
    Serial.println(F("LAKSHIV TECHNOLOGIES & SERVICES PVT LTD."));
    Serial.println(F("SUPERMARKET PRODUCT PRICE SYSTEM"));
    Serial.println(F("=========================================="));

    // Initialize E-Paper power-enable and hardware SPI.
    QRBegin();
    SPI.begin();

    // Load product prices from LittleFS. Defaults are used if no file exists.
    ProductStoreBegin();

    // Register both customer and admin website routes once.
    RegisterWebRoutes();

    if (ConnectToWiFi()) {
        StartWebServer();
        QRUpdateIfNeeded();
    }

    Serial.println(F("Setup completed."));
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED) {
        if (!IsWebServerStarted()) {
            StartWebServer();
            QRUpdateIfNeeded();
        }

        HandleWebServerClient();

        // A DHCP lease can change after reconnection; refresh QR when URL changes.
        if (millis() - lastQrIpCheck >= QR_IP_CHECK_MS) {
            lastQrIpCheck = millis();
            QRUpdateIfNeeded();
        }
    } else {
        StopWebServer();

        if (millis() - lastReconnectAttempt >= WIFI_RETRY_MS) {
            lastReconnectAttempt = millis();
            Serial.println(F("Wi-Fi disconnected. Reconnecting..."));
            WiFi.reconnect();
        }
    }

    delay(2);
    yield();
}
