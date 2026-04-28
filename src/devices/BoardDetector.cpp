#include "BoardDetector.h"

#include <memory>

BoardDetector::BoardDetector(Print *logger)
    : logger(logger) {}

bool BoardDetector::testI2CConfiguration(const BoardConfig &config)
{
    Wire.begin(config.sdaPin, config.sclPin);

    delay(50);

    bool foundDevice = false;
    const uint8_t addresses[] = {0x0, 0x3C, 0x3D, 0x78, 0x7A};

    for (const auto &addr : addresses)
    {
        Wire.beginTransmission(addr);
        auto error = Wire.endTransmission();

        if (error == 0)
        {
            foundDevice = true;
            logger->print("Device at 0x");
            logger->println(addr, HEX);
            break;
        }
    }

    logger->print("I2C scan: ");
    logger->print(foundDevice ? "found!" : "not found.");

    logger->print(" board type=");
    logger->print((uint8_t)config.type);
    logger->print(" (SDA=");
    logger->print(config.sdaPin);
    logger->print(", SCL=");
    logger->print(config.sclPin);
    logger->println(")");

    bool i2cWorking = foundDevice;

    return i2cWorking;
}

BoardConfig *const BoardDetector::detectBoard()
{
    logger->println("Board detection...");

    for (const auto &config : BOARD_CONFIGS)
    {
        if (testI2CConfiguration(config))
        {
            logger->print("Board type=");
            logger->println((uint8_t)config.type);
            return new BoardConfig(config);
        }

        logger->print("Failed for: type=");
        logger->println((uint8_t)config.type);
    }

    logger->print("No board detected, fallback: type=");
    auto fallbackConf = BOARD_CONFIGS[0];
    logger->println((uint8_t)fallbackConf.type);
    return new BoardConfig(fallbackConf);
}
