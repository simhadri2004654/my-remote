#include "token.h"
#include "storage.h"

#include <ESP8266WiFi.h>
#include <stdio.h>

bool tokenGenerateNext(String &id, String &nonce)
{
    uint64_t sequence = 0;
    if (!storageAllocateNextId(sequence))
        return false;

    id = uint64ToString(sequence);

    // Use two ESP8266 hardware-random values for a 64-bit unpredictable suffix.
    // The monotonic ID guarantees uniqueness; the nonce makes guessing harder.
    const uint32_t randomHigh = ESP.random();
    const uint32_t randomLow = ESP.random();

    char nonceBuffer[17];
    snprintf(nonceBuffer, sizeof(nonceBuffer), "%08lx%08lx",
             static_cast<unsigned long>(randomHigh),
             static_cast<unsigned long>(randomLow));

    nonce = String(nonceBuffer);
    return (id.length() > 0 && nonce.length() == 16);
}
