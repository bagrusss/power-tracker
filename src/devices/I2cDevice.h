#pragma once

class I2cDevice
{
public:
    virtual const uint8_t getAddress() const = 0;
};
