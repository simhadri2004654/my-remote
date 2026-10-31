#pragma once

#include <Arduino.h>

void webServerBegin();
void webServerStop();
void webServerHandleClient();

// Called only after the corresponding QR has finished drawing on E-Paper.
void webServerSetActiveToken(const String &token);
void webServerInvalidateActiveToken();

// Returns true once for each successful scan request.
bool webServerTakeQrRefreshRequest();
