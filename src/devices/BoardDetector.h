#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "BoardConfig.h"

class BoardDetector
{
public:
    BoardDetector(Print *const logger);

    BoardConfig *const detectBoard();

private:
    Print *const logger;

    bool testI2CConfiguration(const BoardConfig &config);
};