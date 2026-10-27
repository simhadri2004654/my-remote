#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// IR RECEIVER
// =====================================================

// IR receiver OUT -> GPIO14 (D5 on NodeMCU-style boards)
static const uint16_t IR_RECEIVE_PIN = 14;

// Suppress repeated identical frames while a button is held.
static const uint32_t REPEAT_TIMEOUT_MS = 350;

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

// =====================================================
// I2C OLED
// Assumes SSD1306, 128 x 64 pixels
// =====================================================

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS  0x3C
#define OLED_RESET    -1

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

// =====================================================
// PREVIOUS IR FRAME INFORMATION
// =====================================================

static bool previousValid = false;
static decode_type_t previousProtocol = UNKNOWN;
static uint16_t previousAddress = 0;
static uint32_t previousCommand = 0;
static uint32_t previousFrameTime = 0;

// =====================================================
// DISPLAY MESSAGE CENTERED ON OLED
// Centers text horizontally and vertically.
// =====================================================

void showOnOLED(const char *message)
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);

    String text(message);
    text.replace('\n', ' ');
    text.replace('\r', ' ');
    text.trim();

    const uint8_t CHARS_PER_LINE = SCREEN_WIDTH / 6;
    const uint8_t LINE_HEIGHT = 10;
    const uint8_t MAX_LINES = SCREEN_HEIGHT / LINE_HEIGHT;

    String lines[MAX_LINES];
    uint8_t lineCount = 0;
    uint16_t start = 0;

    // Wrap long messages across OLED lines.
    while (start < text.length() && lineCount < MAX_LINES)
    {
        uint16_t remaining = text.length() - start;

        if (remaining <= CHARS_PER_LINE)
        {
            lines[lineCount] = text.substring(start);
            lines[lineCount].trim();
            lineCount++;
            break;
        }

        uint16_t maxEnd = start + CHARS_PER_LINE;

        int splitAt = text.lastIndexOf(' ', maxEnd);

        // If there is no suitable space, split the long word.
        if (splitAt <= start)
        {
            splitAt = maxEnd;
        }

        lines[lineCount] = text.substring(start, splitAt);
        lines[lineCount].trim();

        lineCount++;
        start = splitAt;

        // Skip spaces before the next line.
        while (start < text.length() &&
               text.charAt(start) == ' ')
        {
            start++;
        }
    }

    // Center all lines vertically.
    int y = (SCREEN_HEIGHT - (lineCount * LINE_HEIGHT)) / 2;

    for (uint8_t i = 0; i < lineCount; i++)
    {
        // Default font is approximately 6 pixels per character.
        int textWidth = lines[i].length() * 6;

        // Center each line horizontally.
        int x = (SCREEN_WIDTH - textWidth) / 2;

        if (x < 0)
        {
            x = 0;
        }

        display.setCursor(x, y);
        display.print(lines[i]);

        y += LINE_HEIGHT;
    }

    display.display();
}

// =====================================================
// PRINT MESSAGE ON SERIAL MONITOR AND OLED
// =====================================================

void printButtonMessage(const char *message)
{
    Serial.println(message);
    showOnOLED(message);
}

// =====================================================
// MAP RC5 COMMANDS TO BUTTON NAMES
// =====================================================

void printRC5Button(uint32_t command)
{
    switch (command)
    {
        case 0x00: printButtonMessage("0 is pressed"); break;
        case 0x01: printButtonMessage("1 is pressed"); break;
        case 0x02: printButtonMessage("2 is pressed"); break;
        case 0x03: printButtonMessage("3 is pressed"); break;
        case 0x04: printButtonMessage("4 is pressed"); break;
        case 0x05: printButtonMessage("5 is pressed"); break;
        case 0x06: printButtonMessage("6 is pressed"); break;
        case 0x07: printButtonMessage("7 is pressed"); break;
        case 0x08: printButtonMessage("8 is pressed"); break;
        case 0x09: printButtonMessage("9 is pressed"); break;

        case 0x0C:
            printButtonMessage("Power button pressed");
            break;

        case 0x0D:
            printButtonMessage("Mute button pressed");
            break;

        case 0x10:
            printButtonMessage("Volume increased button pressed");
            break;

        case 0x11:
            printButtonMessage("Volume decreased button pressed");
            break;

        case 0x12:
            printButtonMessage("Brightness increased button pressed");
            break;

        case 0x13:
            printButtonMessage("Brightness decreased button pressed");
            break;

        case 0x20:
            printButtonMessage("Channel increased button pressed");
            break;

        case 0x21:
            printButtonMessage("Channel decreased button pressed");
            break;

        case 0xF:
            printButtonMessage("Channel Toggle button pressed");
            break;

        case 0x24:
            printButtonMessage("Spatial button pressed");
            break;

        case 0x23:
            printButtonMessage("Stereo button pressed");
            break;

        case 0xA:
            printButtonMessage("Pict button pressed");
            break;

        case 0xB:
            printButtonMessage("Sound button pressed");
            break;

        case 0x2E:
            printButtonMessage("Menu button pressed");
            break;

        case 0x2B:
            printButtonMessage("Menu incresed button pressed");
            break;

        case 0x2C:
            printButtonMessage("Menu decreased button pressed");
            break;

        case 0x29:
            printButtonMessage("Install button pressed");
            break;

        case 0x38:
            printButtonMessage("AV button pressed");
            break;

        default:
        {
            Serial.print("RC5 button detected, command 0x");
            Serial.println(command, HEX);

            String message = "Unknown RC5 cmd Code: 0x";
            message += String(command, HEX);

            showOnOLED(message.c_str());
            break;
        }
    }
}

// =====================================================
// HANDLE ALL DECODABLE PROTOCOLS
// =====================================================

void handleIRResult(const decode_results &result)
{
    uint32_t now = millis();
    uint32_t command = (uint32_t)result.command;
    uint16_t address = (uint16_t)result.address;

    // Suppress consecutive identical frames during a hold.
    bool duplicate =
        previousValid &&
        result.decode_type == previousProtocol &&
        address == previousAddress &&
        command == previousCommand &&
        (uint32_t)(now - previousFrameTime) < REPEAT_TIMEOUT_MS;

    previousValid = true;
    previousProtocol = result.decode_type;
    previousAddress = address;
    previousCommand = command;
    previousFrameTime = now;

    if (duplicate)
    {
        return;
    }

    // Standard RC5 commands and RC5X commands.
    if (result.decode_type == RC5 ||
        result.decode_type == RC5X)
    {
        printRC5Button(command);
        return;
    }

    // Other decodable protocols.
    Serial.print("IR button detected - Protocol: ");
    Serial.print(typeToString(result.decode_type));
    Serial.print(", Address: 0x");
    Serial.print(address, HEX);
    Serial.print(", Command: 0x");
    Serial.println(command, HEX);

    String message = "Other IR signal Address: 0x";
    message += String(address, HEX);
    message += " Command: 0x";
    message += String(command, HEX);

    showOnOLED(message.c_str());
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    // SDA = GPIO4 (D2)
    // SCL = GPIO5 (D1)
    Wire.begin(4, 5);

    // Initialise OLED.
    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            OLED_ADDRESS,
            true,
            false))
    {
        Serial.println("OLED initialization failed!");

        while (true)
        {
            yield();
        }
    }

    // Display centered startup message.
    showOnOLED("IR receiver ready Press any button");

    // Start the IR receiver.
    irrecv.enableIRIn();

    Serial.println();
    Serial.println("IR receiver ready. Press any remote button.");
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
    if (irrecv.decode(&results))
    {
        handleIRResult(results);
        irrecv.resume();
    }

    yield();
}