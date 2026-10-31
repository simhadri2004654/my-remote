#pragma once

#include <Arduino.h>

// Generates a never-reused 64-bit decimal ID plus a 64-bit random nonce.
// The combined token is used by the web server to validate one successful scan.
bool tokenGenerateNext(String &id, String &nonce);
