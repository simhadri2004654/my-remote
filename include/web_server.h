#pragma once

#include <ESP8266WebServer.h>

extern ESP8266WebServer webServer;

void RegisterWebRoutes();
void StartWebServer();
void StopWebServer();
bool IsWebServerStarted();
void HandleWebServerClient();
