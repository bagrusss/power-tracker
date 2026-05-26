#pragma once
#include <Arduino.h>

// Размер общего буфера: 32 байта достаточно для всех строк в Titles/JsonKeys
#define STR_UTILS_BUF_SIZE 32

class StrUtils
{
public:
    // Читает PROGMEM-строку во внутренний буфер и возвращает указатель
    static const char *read(const __FlashStringHelper *flash)
    {
        strcpy_P(_buf, (PGM_P)flash);
        return _buf;
    }

    // Перегрузка для String — удобно, если нужно скопировать в String
    static String readStr(const __FlashStringHelper *flash)
    {
        strcpy_P(_buf, (PGM_P)flash);
        return String(_buf);
    }

private:
    static char _buf[STR_UTILS_BUF_SIZE];
};
