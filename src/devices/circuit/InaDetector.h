#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <map>
#include <memory>
#include <GyverDBFile.h>

#include "devices/BoardConfig.h"

enum class InaType
{
    UNKNOWN,
    INA219,
    INA226,
    INA231
};

class PowerMonitor;

class InaDetector
{
public:
    InaDetector(
        Print *const logger,
        BoardConfig *const config);

    std::map<uint8_t, std::unique_ptr<PowerMonitor>> *detectDevices(const uint8_t &startAddr, const uint8_t &endAddr, GyverDBFile *const globalDb) const;

private:
    Print *const logger;
    BoardConfig *const config;

    PowerMonitor *createInaMonitor(const InaType &type, const uint8_t &address, GyverDBFile *const globalDb) const;
    void initDeviceSettings(const uint8_t &address, GyverDBFile *const db, const InaType &inaType) const;

    static bool isDevicePresent(const uint8_t &i2cAddress);
    static uint16_t readRegister(const uint8_t &i2cAddress, const uint8_t &reg);
    const InaType detectType(const uint8_t &i2cAddress) const;
};
