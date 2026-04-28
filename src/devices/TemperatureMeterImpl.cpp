#include "devices/TemperatureMeterImpl.h"

TemperatureMeterImpl::TemperatureMeterImpl(const uint8_t &pin) : ntc(GyverNTC(pin, 100000, 3950, 25, 100000)) {}

float TemperatureMeterImpl::getTemperature()
{
    return ntc.getTempAverage();
}