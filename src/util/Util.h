#pragma once
#include <WString.h>

template <typename T>
String withUnit(const T value, const char *unit, const char *altUnit = nullptr)
{
    auto v = value;
    auto u = unit;
    if (altUnit && value > 1000)
    {
        v /= 1000;
        u = altUnit;
    }
    auto res = String(v);
    res += ' ';
    res += u;
    return res;
}