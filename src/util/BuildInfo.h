#pragma once

#include <Arduino.h>

#ifndef BUILD_VERSION
#define BUILD_VERSION "1.0.2"
#endif

#ifndef BUILD_PLATFORM
#define BUILD_PLATFORM "esp8266"
#endif

#ifndef BUILD_TYPE
#define BUILD_TYPE "debug"
#endif

#ifndef BUILD_NUMBER
#define BUILD_NUMBER 5
#endif

namespace BuildInfo {
    const char* getVersion();
    const char* getPlatform();
    const char* getBuildType();
    int getBuildNumber();
    String getFullVersion();
}
