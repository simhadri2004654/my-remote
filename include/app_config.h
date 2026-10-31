#pragma once

// ============================================================
// LAKSHIV PRICE SYSTEM - PROJECT CONFIGURATION
// Replace the Wi-Fi credentials and administrator password.
// ============================================================

#define WIFI_SSID       "Lakshiv"
#define WIFI_PASSWORD   "kanGan#072010"

#define ADMIN_USERNAME  "admin"
#define ADMIN_PASSWORD  "Lakshiv"

// Waveshare 3.5-inch E-Paper Module (G) driver-board pins.
// GPIO numbers are used rather than D-pin aliases.
#define EPD_CS_PIN      5    // D1 / GPIO5
#define EPD_DC_PIN      4    // D2 / GPIO4
#define EPD_RST_PIN     16   // D0 / GPIO16
#define EPD_BUSY_PIN    12   // D6 / GPIO12
#define EPD_PWR_PIN     2    // D4 / GPIO2 - check board wiring/boot strap

#define QR_VERSION       2
#define QR_BUFFER_SIZE   256
#define QR_SCALE         4
#define QR_QUIET_ZONE    4
#define QR_X             12
#define QR_Y             42
#define QR_DISPLAY_SIZE  132

#define CUSTOMER_REFRESH_MS 15000UL
#define WIFI_RETRY_MS       10000UL
#define QR_IP_CHECK_MS       3000UL
