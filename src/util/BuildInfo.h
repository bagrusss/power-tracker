#pragma once

#include <Arduino.h>

#ifndef BUILD_VERSION
#define BUILD_VERSION "1.0.5"
#endif

#ifndef BUILD_PLATFORM
#define BUILD_PLATFORM "ESP8266"
#endif

#ifndef BUILD_TYPE
#define BUILD_TYPE "debug"
#endif

#ifndef BUILD_NUMBER
#define BUILD_NUMBER 8
#endif

namespace BuildInfo {
    const char* getVersion();
    const char* getPlatform();
    const char* getBuildType();
    int getBuildNumber();
    void getFullVersion(char *buf, const size_t &size);
}
