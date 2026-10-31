#pragma once

#include <Arduino.h>

struct Product {
    const char* id;
    const char* name;
    const char* unit;
    float price;
};

extern Product gProducts[];
extern const uint8_t PRODUCT_COUNT;

// Mount LittleFS and load saved prices. Returns false if LittleFS cannot mount.
bool ProductStoreBegin();

// Save all current prices to /prices.txt in LittleFS.
bool SavePrices();

// Find a product by its stable ID. Returns -1 when not found.
int FindProductById(const String& id);

// Build the product JSON response for the customer/admin web pages.
String BuildProductsJson();
