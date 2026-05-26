#pragma once
#include <Arduino.h>
#include <stdlib.h>

// Буфер для withUnit - 32 байт достаточно для "99999.99 mWh" и подобных значений
#define WITH_UNIT_BUF_SIZE 32

template <typename T>
const char *withUnit(const T value, const char *unit, const char *altUnit = nullptr)
{
    static char buf[WITH_UNIT_BUF_SIZE];
    auto v = value;
    auto u = unit;
    if (altUnit && value > 1000)
    {
        v /= 1000;
        u = altUnit;
    }
    dtostrf(v, 0, 2, buf);
    size_t len = strlen(buf);
    buf[len] = ' ';
    buf[len + 1] = '\0';
    strncat(buf, u, WITH_UNIT_BUF_SIZE - len - 2);
    return buf;
}