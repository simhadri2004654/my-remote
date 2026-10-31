#include "web_server.h"
#include "customer.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

namespace
{
    ESP8266WebServer server(80);
    String activeToken;
    bool activeTokenValid = false;
    bool qrRefreshPending = false;
    bool routesRegistered = false;
    bool serverRunning = false;

    void handleScan()
    {
        const String receivedId = server.arg("id");
        const String receivedNonce = server.arg("r");
        const String receivedToken = receivedId + "-" + receivedNonce;

        // ESP8266WebServer processes requests serially. Clear validity BEFORE
        // responding so a duplicate request cannot consume this token again.
        if (!activeTokenValid || receivedId.length() == 0 ||
            receivedNonce.length() != 16 || receivedToken != activeToken)
        {
            Serial.print(F("Rejected old/invalid QR token: "));
            Serial.println(receivedId);
            customerSendExpiredPage(server);
            return;
        }

        activeTokenValid = false;
        qrRefreshPending = true;

        Serial.print(F("Accepted QR ID: "));
        Serial.println(receivedId);
        Serial.println(F("Old token invalidated; customer page sent."));

        // Respond first. main.cpp refreshes the E-Paper after handleClient()
        // returns, so the phone does not wait for the e-paper refresh cycle.
        customerSendPage(server, receivedToken);
    }

    void handleHome()
    {
        customerSendHomePage(server);
    }
}

void webServerBegin()
{
    if (!routesRegistered)
    {
        server.on("/", HTTP_GET, handleHome);
        server.on("/scan", HTTP_GET, handleScan);
        server.onNotFound([]()
        {
            server.send(404, "text/plain", "Not found");
        });
        routesRegistered = true;
    }

    if (!serverRunning)
    {
        server.begin();
        serverRunning = true;
        Serial.println(F("HTTP server started on port 80."));
    }
}

void webServerStop()
{
    if (serverRunning)
    {
        server.stop();
        serverRunning = false;
    }

    activeTokenValid = false;
    qrRefreshPending = false;
}

void webServerHandleClient()
{
    if (serverRunning)
        server.handleClient();
}

void webServerSetActiveToken(const String &token)
{
    activeToken = token;
    activeTokenValid = (token.length() > 0);
    Serial.print(F("New active QR ID: "));
    Serial.println(activeToken);
}

void webServerInvalidateActiveToken()
{
    activeTokenValid = false;
}

bool webServerTakeQrRefreshRequest()
{
    if (!qrRefreshPending)
        return false;

    qrRefreshPending = false;
    return true;
}
