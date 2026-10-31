#include "product_store.h"

#include <LittleFS.h>
#include <math.h>
#include <stdlib.h>

Product gProducts[] = {
    {"banana", "Banana", "dozen", 100.00f},
    {"apple",  "Apple",  "kg",    150.00f},
    {"tomato", "Tomato", "kg",     40.00f},
    {"potato", "Potato", "kg",     30.00f},
    {"onion",  "Onion",  "kg",     45.00f}
};

const uint8_t PRODUCT_COUNT = sizeof(gProducts) / sizeof(gProducts[0]);

static bool gFileSystemReady = false;

bool SavePrices()
{
    if (!gFileSystemReady) {
        Serial.println(F("ERROR: LittleFS is not available; prices were not saved."));
        return false;
    }

    File file = LittleFS.open("/prices.txt", "w");
    if (!file) {
        Serial.println(F("ERROR: Cannot open /prices.txt for writing."));
        return false;
    }

    for (uint8_t i = 0; i < PRODUCT_COUNT; ++i) {
        file.print(gProducts[i].id);
        file.print('=');
        file.println(gProducts[i].price, 2);
    }

    file.flush();
    const bool success = (file.getWriteError() == 0);
    file.close();

    if (!success) {
        Serial.println(F("ERROR: Writing /prices.txt failed."));
    }
    return success;
}

static void LoadPrices()
{
    if (!gFileSystemReady) return;

    if (!LittleFS.exists("/prices.txt")) {
        Serial.println(F("No saved price file. Creating defaults."));
        SavePrices();
        return;
    }

    File file = LittleFS.open("/prices.txt", "r");
    if (!file) {
        Serial.println(F("ERROR: Cannot open /prices.txt for reading. Defaults remain in RAM."));
        return;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();

        const int separator = line.indexOf('=');
        if (separator <= 0) continue;

        const String id = line.substring(0, separator);
        String priceText = line.substring(separator + 1);
        priceText.trim();

        char* endPointer = nullptr;
        const double value = strtod(priceText.c_str(), &endPointer);

        if (priceText.length() == 0 ||
            endPointer == priceText.c_str() ||
            *endPointer != '\0' ||
            !isfinite(value) ||
            value < 0.0 ||
            value > 1000000.0) {
            Serial.println(F("Ignoring invalid price record."));
            continue;
        }

        const int index = FindProductById(id);
        if (index >= 0) {
            gProducts[index].price = static_cast<float>(value);
        }
    }

    file.close();
    Serial.println(F("Product prices loaded from LittleFS."));
}

bool ProductStoreBegin()
{
    gFileSystemReady = LittleFS.begin();
    if (!gFileSystemReady) {
        Serial.println(F("ERROR: LittleFS mount failed. Price updates cannot persist."));
        return false;
    }

    Serial.println(F("LittleFS mounted."));
    LoadPrices();
    return true;
}

int FindProductById(const String& id)
{
    for (uint8_t i = 0; i < PRODUCT_COUNT; ++i) {
        if (id == gProducts[i].id) return static_cast<int>(i);
    }
    return -1;
}

String BuildProductsJson()
{
    String json;
    json.reserve(420);
    json = '[';

    for (uint8_t i = 0; i < PRODUCT_COUNT; ++i) {
        if (i > 0) json += ',';

        // IDs, names and units are fixed constants in this firmware.
        json += F("{\"id\":\"");
        json += gProducts[i].id;
        json += F("\",\"name\":\"");
        json += gProducts[i].name;
        json += F("\",\"price\":");
        json += String(gProducts[i].price, 2);
        json += F(",\"unit\":\"");
        json += gProducts[i].unit;
        json += F("\"}");
    }

    json += ']';
    return json;
}
