#pragma once

class TemperatureMeter
{
public:
    virtual ~TemperatureMeter() = default;
    virtual float getTemperature() = 0;
};