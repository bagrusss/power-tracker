#pragma once

#include "devices/circuit/InaDetector.h"
#include <LittleFS.h>

namespace Prefs
{

    constexpr const char *INA_DEVICES = "UNKNOWN;INA_219;INA_226;INA_231";
    constexpr const char *POWER_STRATEGIES = "Battery;Power Source";

}