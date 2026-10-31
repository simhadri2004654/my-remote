#pragma once

#include <Arduino.h>

// ==================== Wi-Fi ====================
// Replace these with your local Wi-Fi credentials.
#define WIFI_SSID       "Lakshiv"
#define WIFI_PASSWORD   "kanGan#072010"

// ==================== E-Paper pins ====================
// ESP8266MOD / ESP-12E GPIO mapping used by this project.
// SPI: DIN=GPIO13, SCLK=GPIO14.
#define EPD_CS_PIN      5   // GPIO5  (D1 on NodeMCU-style boards)
#define EPD_DC_PIN      4   // GPIO4  (D2)
#define EPD_RST_PIN     16  // GPIO16 (D0)
#define EPD_BUSY_PIN    12  // GPIO12 (D6)
#define EPD_PWR_PIN     2   // GPIO2  (D4), if your adapter board has PWR

// ==================== QR configuration ====================
// Version 5 / ECC-M accommodates IPv4 URL + 64-bit ID + 64-bit nonce.
static constexpr uint8_t QR_VERSION = 5;
static constexpr uint8_t QR_SCALE = 3;
static constexpr uint8_t QR_QUIET_ZONE = 4;

// Version 5 has 37 x 37 data modules. With 4 quiet modules on each
// edge and scale 3, the printed QR occupies 135 x 135 pixels.
static constexpr int16_t QR_X = 8;
static constexpr int16_t QR_Y = 40;

// Reserve IDs in chunks to avoid writing flash on every scan.
// On a power restart, any unused IDs in the previous block are skipped.
static constexpr uint64_t QR_ID_BLOCK_SIZE = 65536ULL;

static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 30000UL;
static constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 10000UL;

