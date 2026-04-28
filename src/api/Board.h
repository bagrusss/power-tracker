#pragma once

#include "devices/BoardConfig.h"

class Board
{
public:
    virtual ~Board() = default;
    virtual void reboot() = 0;
    virtual BoardConfig *const getConfig() const = 0;
    virtual uint32_t getFreeHeap() const = 0;
    virtual void setI2CFreq(const uint32_t &freqHz) = 0;
};
