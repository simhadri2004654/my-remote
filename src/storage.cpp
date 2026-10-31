#include "storage.h"
#include "app_config.h"

#include <LittleFS.h>
#include <stdlib.h>
#include <stdio.h>

namespace
{
    const char *SLOT_A = "/qr_state_a.txt";
    const char *SLOT_B = "/qr_state_b.txt";

    uint64_t nextId = 1;
    uint64_t reservedEnd = 1; // Exclusive end of the reserved ID block.
    uint64_t stateGeneration = 0;
    bool storageReady = false;

    struct SlotState
    {
        bool valid = false;
        uint64_t generation = 0;
        uint64_t reservedEnd = 0;
    };

    String uint64Text(uint64_t value)
    {
        char buffer[21];
        char *p = &buffer[20];
        *p = '\0';
        do
        {
            const uint8_t digit = static_cast<uint8_t>(value % 10ULL);
            *--p = static_cast<char>('0' + digit);
            value /= 10ULL;
        }
        while (value != 0ULL);
        return String(p);
    }

    bool parseUnsigned64(const String &text, uint64_t &value)
    {
        if (text.length() == 0)
            return false;

        for (size_t i = 0; i < text.length(); ++i)
        {
            if (text[i] < '0' || text[i] > '9')
                return false;
        }

        char *endPtr = nullptr;
        const unsigned long long parsed = strtoull(text.c_str(), &endPtr, 10);
        if (endPtr == text.c_str() || *endPtr != '\0')
            return false;

        value = static_cast<uint64_t>(parsed);
        return true;
    }

    uint32_t fnv1a(const String &text)
    {
        uint32_t hash = 2166136261UL;
        for (size_t i = 0; i < text.length(); ++i)
        {
            hash ^= static_cast<uint8_t>(text[i]);
            hash *= 16777619UL;
        }
        return hash;
    }

    bool readSlot(const char *path, SlotState &state)
    {
        state.valid = false;
        if (!LittleFS.exists(path))
            return false;

        File file = LittleFS.open(path, "r");
        if (!file)
            return false;

        String line = file.readStringUntil('\n');
        file.close();
        line.trim();

        const int comma1 = line.indexOf(',');
        const int comma2 = line.indexOf(',', comma1 + 1);
        const int comma3 = line.indexOf(',', comma2 + 1);
        if (comma1 < 0 || comma2 < 0 || comma3 < 0)
            return false;

        if (line.substring(0, comma1) != "AQR1")
            return false;

        const String generationText = line.substring(comma1 + 1, comma2);
        const String endText = line.substring(comma2 + 1, comma3);
        const String checksumText = line.substring(comma3 + 1);

        uint64_t generation = 0;
        uint64_t endValue = 0;
        if (!parseUnsigned64(generationText, generation) ||
            !parseUnsigned64(endText, endValue) ||
            generation == 0 || endValue <= 1 || checksumText.length() != 8)
            return false;

        char *hexEnd = nullptr;
        const unsigned long storedChecksum = strtoul(checksumText.c_str(), &hexEnd, 16);
        if (hexEnd == checksumText.c_str() || *hexEnd != '\0')
            return false;

        const String payload = line.substring(0, comma3);
        if (fnv1a(payload) != static_cast<uint32_t>(storedChecksum))
            return false;

        state.valid = true;
        state.generation = generation;
        state.reservedEnd = endValue;
        return true;
    }

    bool writeSlot(uint64_t generation, uint64_t newExclusiveEnd)
    {
        const String payload = String("AQR1,") + uint64Text(generation) + "," + uint64Text(newExclusiveEnd);
        const uint32_t checksum = fnv1a(payload);

        char checksumText[9];
        snprintf(checksumText, sizeof(checksumText), "%08lx", static_cast<unsigned long>(checksum));

        // Alternate A/B slots. The currently newest valid slot is never
        // overwritten until the other slot has been written and verified.
        const char *target = (generation & 1ULL) ? SLOT_A : SLOT_B;
        File file = LittleFS.open(target, "w");
        if (!file)
        {
            Serial.println(F("ERROR: Cannot open QR state slot for writing."));
            return false;
        }

        file.print(payload);
        file.print(',');
        file.print(checksumText);
        file.print('\n');
        file.flush();
        const bool writeOk = (file.getWriteError() == 0);
        file.close();
        if (!writeOk)
            return false;

        SlotState verify;
        if (!readSlot(target, verify) ||
            verify.generation != generation ||
            verify.reservedEnd != newExclusiveEnd)
        {
            Serial.println(F("ERROR: QR state slot verification failed."));
            return false;
        }

        return true;
    }

    bool reserveNextBlock()
    {
        const uint64_t maxValue = UINT64_MAX;
        if (nextId >= maxValue || QR_ID_BLOCK_SIZE > (maxValue - nextId) ||
            stateGeneration >= maxValue - 1ULL)
        {
            Serial.println(F("FATAL: 64-bit QR counter is exhausted. It will not wrap or reuse IDs."));
            return false;
        }

        const uint64_t newEnd = nextId + QR_ID_BLOCK_SIZE;
        const uint64_t newGeneration = stateGeneration + 1ULL;

        // Persist and verify the reserved range BEFORE any ID from it is issued.
        if (!writeSlot(newGeneration, newEnd))
            return false;

        stateGeneration = newGeneration;
        reservedEnd = newEnd;

        Serial.print(F("Reserved QR ID range ending before: "));
        Serial.println(uint64Text(reservedEnd));
        return true;
    }
}

String uint64ToString(uint64_t value)
{
    return uint64Text(value);
}

bool storageBegin()
{
    storageReady = false;

    if (!LittleFS.begin())
    {
        Serial.println(F("ERROR: LittleFS mount failed. Counter safety cannot be guaranteed."));
        return false;
    }

    SlotState a;
    SlotState b;
    const bool hasA = LittleFS.exists(SLOT_A);
    const bool hasB = LittleFS.exists(SLOT_B);
    const bool validA = readSlot(SLOT_A, a);
    const bool validB = readSlot(SLOT_B, b);

    if (!hasA && !hasB)
    {
        // Fresh first boot only. If either slot exists but is corrupt, never
        // silently restart from 1 because that could repeat a previous QR.
        nextId = 1;
        reservedEnd = 1;
        stateGeneration = 0;
        Serial.println(F("First run: initializing QR counter at ID 1."));
    }
    else if (validA || validB)
    {
        SlotState latest;
        if (validA && validB)
        {
            if (a.generation == b.generation && a.reservedEnd != b.reservedEnd)
            {
                Serial.println(F("FATAL: QR counter slots conflict."));
                return false;
            }
            latest = (a.generation >= b.generation) ? a : b;
        }
        else
        {
            latest = validA ? a : b;
        }

        // Skip all IDs in the last reserved block after reboot. Some may not
        // have been used, but skipping is safer than ever reusing an old ID.
        stateGeneration = latest.generation;
        nextId = latest.reservedEnd;
        reservedEnd = latest.reservedEnd;

        Serial.print(F("Recovered next safe QR ID: "));
        Serial.println(uint64Text(nextId));
    }
    else
    {
        Serial.println(F("FATAL: QR counter slots are missing/corrupt; refusing to reuse IDs."));
        return false;
    }

    // Reserve a fresh block at startup before displaying any QR.
    if (!reserveNextBlock())
        return false;

    storageReady = true;
    return true;
}

bool storageAllocateNextId(uint64_t &id)
{
    if (!storageReady)
        return false;

    if (nextId >= reservedEnd && !reserveNextBlock())
        return false;

    id = nextId;
    ++nextId;
    return true;
}
