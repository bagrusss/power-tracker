#pragma once

#include <cstdint>

class SensorStatusListener {
public:
    virtual ~SensorStatusListener() = default;
    virtual void onSensorStatusChanged(const uint8_t address, bool isBusy) = 0;
};