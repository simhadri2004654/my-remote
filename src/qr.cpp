
#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <GxEPD2_4C.h>
#include <qrcode.h>

#include "qr.h"
#include "app_config.h"
#include "token.h"
#include "web_server.h"


// ============================================================
// 1. WAVESHARE 3.5-INCH E-PAPER DRIVER
// Panel: GDEM035F51
// Resolution: 384 x 184
// Display: Four colours
// ============================================================

GxEPD2_4C<
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
// 2. QR CODE GLOBAL OBJECTS
// ============================================================

namespace
{
    QRCode qrCode;

    // QR Version 5 = 37 x 37 modules.
    // Required QR module buffer = 172 bytes.
    // 256 bytes provides sufficient space.

    uint8_t qrBuffer[256];


    // ========================================================
    // 3. DRAW QR MODULES
    // ========================================================

    void drawQrModules()
    {
        const int16_t moduleCount = qrCode.size;

        const int16_t totalModules =
            moduleCount + (2 * QR_QUIET_ZONE);

        const int16_t totalPixels =
            totalModules * QR_SCALE;


        // Clear QR area with white background.
        // This also creates the four-module quiet zone.

        display.fillRect(
            QR_X,
            QR_Y,
            totalPixels,
            totalPixels,
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


    // ========================================================
    // 4. DRAW COMPLETE E-PAPER LAYOUT
    // ========================================================

    void drawLayout(const String &qrId)
    {
        // Landscape display: 384 x 184 pixels.

        display.setRotation(1);

        display.setFullWindow();

        display.firstPage();

        do
        {
            // Entire background is white.

            display.fillScreen(GxEPD_WHITE);


            // ------------------------------------------------
            // SCAN TO OPEN HEADING
            // ------------------------------------------------

            display.setTextWrap(false);

            display.setTextColor(GxEPD_RED);

            display.setTextSize(2);

            display.setCursor(3,26 );

            display.print(F("   SCAN ME"));


            // ------------------------------------------------
            // QR CODE
            // ------------------------------------------------

            drawQrModules();


            // ------------------------------------------------
            // COMPANY INFORMATION
            // ------------------------------------------------

            // All company text is black.
            // No coloured background is used.

            display.setTextColor(GxEPD_BLACK);

            display.setTextSize(2);

            display.setTextWrap(false);


            // Company line 1

            display.setCursor(164, 52);

            display.print(F("LAKSHIV"));


            // Company line 2

            display.setCursor(164, 76);

            display.print(F("TECHNOLOGIES"));


            // Company line 3

            display.setCursor(164, 100);

            display.print(F("& SERVICES"));


            // Company line 4

            display.setCursor(164, 124);

            display.print(F("PVT LTD,."));


            // ------------------------------------------------
            // CURRENT QR ID
            // ------------------------------------------------

            //display.setTextSize(1);

            //display.setCursor(164, 155);

            //display.print(F("QR ID: "));

            //display.print(qrId);

        }
        while (display.nextPage());


        // Put display controller into hibernate mode.
        // The visible image remains on the E-Paper.

        display.hibernate();
    }

}


// ============================================================
// 5. INITIALIZE E-PAPER
// ============================================================

void qrBegin()
{
    Serial.println();

    Serial.println(F("Initializing Waveshare E-Paper..."));


    // E-Paper power enable.

    pinMode(EPD_PWR_PIN, OUTPUT);

    digitalWrite(EPD_PWR_PIN, HIGH);

    delay(100);


    // Hardware SPI initialization.

    SPI.begin();


    // Initialize E-Paper driver.

    display.init(
        115200,
        true,
        2,
        false
    );


    Serial.println(
        F("Waveshare GDEM035F51 initialized successfully.")
    );
}


// ============================================================
// 6. GENERATE AND DISPLAY NEXT QR
// ============================================================

bool qrGenerateAndDisplayNext()
{
    // --------------------------------------------------------
    // STEP 1: CHECK WIFI CONNECTION
    // --------------------------------------------------------

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(
            F("ERROR: Wi-Fi is disconnected.")
        );

        return false;
    }


    // --------------------------------------------------------
    // STEP 2: GENERATE UNIQUE QR ID AND RANDOM NONCE
    // --------------------------------------------------------

    String newId;

    String nonce;


    if (!tokenGenerateNext(newId, nonce))
    {
        Serial.println(
            F("FATAL: Could not allocate a unique QR ID.")
        );

        return false;
    }


    // --------------------------------------------------------
    // STEP 3: BUILD QR URL
    // --------------------------------------------------------

    // Example:
    // http://192.168.1.100/scan?id=1&r=RANDOM_NONCE

    const String url =
        String("http://") +
        WiFi.localIP().toString() +
        "/scan?id=" +
        newId +
        "&r=" +
        nonce;


    // Combined token used by the web server.

    const String activeToken =
        newId + "-" + nonce;


    Serial.println();

    Serial.println(
        F("====================================")
    );

    Serial.println(
        F("GENERATING NEW QR CODE")
    );

    Serial.print(
        F("QR ID: ")
    );

    Serial.println(newId);

    Serial.print(
        F("QR URL: ")
    );

    Serial.println(url);


    // --------------------------------------------------------
    // STEP 4: GENERATE QR CODE
    // --------------------------------------------------------

    // Version 5, Error Correction Medium.

    const int8_t result = qrcode_initText(
        &qrCode,
        qrBuffer,
        QR_VERSION,
        ECC_MEDIUM,
        url.c_str()
    );


    if (result < 0)
    {
        Serial.println(
            F("ERROR: QR generation failed.")
        );

        Serial.println(
            F("Check QR version and URL length.")
        );

        return false;
    }


    // --------------------------------------------------------
    // STEP 5: REINITIALIZE E-PAPER AFTER HIBERNATION
    // --------------------------------------------------------

    digitalWrite(EPD_PWR_PIN, HIGH);

    delay(100);

    display.init(
        115200,
        false,
        2,
        false
    );


    // --------------------------------------------------------
    // STEP 6: DISPLAY NEW QR AND COMPANY INFORMATION
    // --------------------------------------------------------

    drawLayout(newId);


    // --------------------------------------------------------
    // STEP 7: ACTIVATE NEW TOKEN
    // --------------------------------------------------------

    // Activate only after the new QR has finished drawing.

    webServerSetActiveToken(activeToken);


    Serial.println(
        F("NEW QR DISPLAYED SUCCESSFULLY.")
    );

    Serial.println(
        F("New QR token is now active.")
    );

    Serial.println(
        F("====================================")
    );


    return true;
}
