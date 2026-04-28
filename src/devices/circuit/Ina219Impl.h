#pragma once

#include <INA219.h>

#include "api/PowerMonitor.h"

class Ina219Impl : public PowerMonitor
{
public:
    Ina219Impl(
        const uint8_t address = 0x40,
        const float rShunt = 0.005,
        const float maxCurrent = 10);
    ~Ina219Impl();

    bool begin(const uint8_t &sdaPin,
               const uint8_t &sclPin) override;
    const float maxVoltage() override;

    float measureCurrent() override;
    float measureVoltage() override;
    float measureShuntVoltage() override;
    float measurePower() override;

    InaType getType() const override { return InaType::INA219; }

private:
    INA219 *const ina;
};
