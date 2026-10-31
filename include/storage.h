#pragma once

#include <Arduino.h>
#include <stdint.h>

bool storageBegin();
bool storageAllocateNextId(uint64_t &id);
String uint64ToString(uint64_t value);
