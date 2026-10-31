
#include "qr.h"

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <GxEPD2_4C.h>
#include <qrcode.h>

#include "app_config.h"


// ============================================================
// 1. WAVESHARE E-PAPER DRIVER
// Panel: GDEM035F51
// Resolution: 384 x 184
// ============================================================

static GxEPD2_4C<
    GxEPD2_350c_GDEM035F51,
    GxEPD2_350c_GDEM035F51::HEIGHT
> display(
    GxEPD2_350c_GDEM035F51(
        EPD_CS_PIN,
        EPD_DC_PIN,
        EPD_RST_PIN,
        EPD_BUSY_PIN
    )
);


// ============================================================
// 2. QR CODE VARIABLES
// ============================================================

static QRCode qrCode;
static uint8_t qrBuffer[QR_BUFFER_SIZE];

static String displayedUrl = "";


// Calculate complete QR dimensions including quiet zone.

static int16_t GetQrPixelSize()
{
    return
        (qrCode.size + (2 * QR_QUIET_ZONE))
        * QR_SCALE;
}


// ============================================================
// 3. GENERATE QR CODE
// ============================================================

static bool GenerateQrCode(const String& url)
{
    if (url.length() > 26)
    {
        Serial.println(
            F("ERROR: QR URL is too long for Version 2.")
        );

        return false;
    }

    qrcode_initText(
        &qrCode,
        qrBuffer,
        QR_VERSION,
        ECC_MEDIUM,
        url.c_str()
    );

    if (qrCode.size != 25)
    {
        Serial.println(
            F("ERROR: QR generation failed.")
        );

        return false;
    }

    Serial.println(F("QR generated successfully."));

    return true;
}


// ============================================================
// 4. DRAW SCAN ME HEADING
// Slightly increased size
// Colour: RED
// ============================================================

static void DrawHeading()
{
    display.setTextColor(GxEPD_RED);

    display.setTextSize(2);

    display.setTextWrap(false);

    // SCAN ME = 7 characters.
    // Each character is 12 pixels at text size 2.

    const int16_t headingWidth = 7 * 6 * 2;

    const int16_t headingX =
        QR_X + (GetQrPixelSize() - headingWidth) / 2;

    display.setCursor(
        headingX,
        25
    );

    display.print(F("SCAN ME"));
}


// ============================================================
// 5. DRAW QR CODE
// Reduced QR size
// Black and white only
// ============================================================

static void DrawQrModules()
{
    const int16_t qrSize = GetQrPixelSize();

    // White background and quiet zone.

    display.fillRect(
        QR_X,
        QR_Y,
        qrSize,
        qrSize,
        GxEPD_WHITE
    );

    // Draw black QR modules.

    for (uint8_t y = 0; y < qrCode.size; ++y)
    {
        for (uint8_t x = 0; x < qrCode.size; ++x)
        {
            if (qrcode_getModule(&qrCode, x, y))
            {
                const int16_t pixelX =
                    QR_X +
                    (x + QR_QUIET_ZONE) * QR_SCALE;

                const int16_t pixelY =
                    QR_Y +
                    (y + QR_QUIET_ZONE) * QR_SCALE;

                display.fillRect(
                    pixelX,
                    pixelY,
                    QR_SCALE,
                    QR_SCALE,
                    GxEPD_BLACK
                );
            }
        }
    }
}


// ============================================================
// 6. DRAW COMPANY NAME
// Increased size: 2
// Colour: BLACK
// ============================================================

static void DrawCompanyName()
{
    display.setTextColor(GxEPD_BLACK);

    display.setTextSize(2);

    display.setTextWrap(false);

    const int16_t textX = 145;

    display.setCursor(textX, 52);
    display.print(F("LAKSHIV"));

    display.setCursor(textX, 76);
    display.print(F("TECHNOLOGIES"));

    display.setCursor(textX, 100);
    display.print(F("& SERVICES"));

    display.setCursor(textX, 124);
    display.print(F("PVT LTD."));
}


// ============================================================
// 7. DRAW COMPLETE DISPLAY LAYOUT
// ============================================================

static void DrawFullLayout()
{
    display.setRotation(1);

    display.setFullWindow();

    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);

        // Red heading.

        DrawHeading();

        // Smaller QR code.

        DrawQrModules();

        // Larger company name.

        DrawCompanyName();

    }
    while (display.nextPage());

    display.hibernate();

    Serial.println(
        F("E-Paper display refreshed.")
    );
}


// ============================================================
// 8. INITIALIZE E-PAPER
// ============================================================

void QRBegin()
{
    Serial.println();

    Serial.println(
        F("Initializing Waveshare E-Paper...")
    );

    pinMode(EPD_PWR_PIN, OUTPUT);

    digitalWrite(EPD_PWR_PIN, HIGH);

    delay(100);

    SPI.begin();

    Serial.println(
        F("SPI initialized.")
    );

    Serial.println(
        F("E-Paper power enabled.")
    );
}


// ============================================================
// 9. UPDATE QR WHEN DHCP IP CHANGES
// ============================================================

void QRUpdateIfNeeded()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    // Automatically obtain router-assigned IP.

    const String url =
        String("http://") +
        WiFi.localIP().toString() +
        "/";

    // Avoid refreshing if URL has not changed.

    if (url == displayedUrl)
    {
        return;
    }

    Serial.println();

    Serial.println(
        F("Generating E-Paper QR...")
    );

    Serial.print(F("URL: "));
    Serial.println(url);


    // Generate QR data.

    if (!GenerateQrCode(url))
    {
        return;
    }


    // Enable E-Paper.

    digitalWrite(EPD_PWR_PIN, HIGH);

    delay(200);


    // Initialize display.

    display.init(
        115200,
        true,
        2,
        false
    );


    // Draw QR and company information.

    DrawFullLayout();


    // Remember displayed URL.

    displayedUrl = url;

    Serial.println(
        F("QR display update completed.")
    );
}
