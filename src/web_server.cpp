#include "web_server.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "admin.h"
#include "customer.h"

ESP8266WebServer webServer(80);
static bool serverStarted = false;
static bool routesRegistered = false;

void RegisterWebRoutes()
{
    if (routesRegistered) return;

    webServer.on("/", HTTP_GET, HandleCustomerPage);
    webServer.on("/api/products", HTTP_GET, HandleProductsApi);
    webServer.on("/admin", HTTP_GET, HandleAdminPage);
    webServer.on("/admin/update", HTTP_POST, HandleAdminUpdate);

    webServer.onNotFound([]() {
        webServer.send(404, "text/plain; charset=utf-8", "Page not found.");
    });

    routesRegistered = true;
}

void StartWebServer()
{
    if (serverStarted || WiFi.status() != WL_CONNECTED) return;

    webServer.begin();
    serverStarted = true;

    const String baseUrl = String("http://") + WiFi.localIP().toString();

    Serial.println();
    Serial.println(F("WEB SERVER STARTED"));
    Serial.print(F("Customer website: "));
    Serial.println(baseUrl + "/");
    Serial.print(F("Admin website:    "));
    Serial.println(baseUrl + "/admin");
}

void StopWebServer()
{
    if (!serverStarted) return;

    webServer.stop();
    serverStarted = false;
    Serial.println(F("Web server stopped."));
}

bool IsWebServerStarted()
{
    return serverStarted;
}

void HandleWebServerClient()
{
    if (serverStarted) webServer.handleClient();
}
