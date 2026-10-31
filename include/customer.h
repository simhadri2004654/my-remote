#pragma once

#include <ESP8266WebServer.h>

void customerSendPage(ESP8266WebServer &server, const String &scanId);
void customerSendExpiredPage(ESP8266WebServer &server);
void customerSendHomePage(ESP8266WebServer &server);
