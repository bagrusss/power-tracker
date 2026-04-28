#pragma once

#include "api/TemperatureMeter.h"

#include <GyverNTC.h>

class TemperatureMeterImpl : public TemperatureMeter
{
public:
    TemperatureMeterImpl(const uint8_t &pin);
    float getTemperature() override;

private:
    GyverNTC ntc;
};