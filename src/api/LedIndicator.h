#pragma once

#include <Arduino.h>

class LedIndicator
{
public:
    LedIndicator(int pin, bool activeLow = true);

    void setBlinkDurations(uint16_t onDuration, uint16_t offDuration);
    void setConstantOn(bool enable);
    void setEnabled(bool enable = true);
    void update();

private:
    const uint8_t pin;
    const bool activeLow;

    bool isEnabled = false;
    bool isConstantOn = false;
    bool ledState = false;
    uint32_t lastBlinkTime = 0;
    uint16_t onDuration = 500;
    uint16_t offDuration = 1500;

    void setOn();
    void setOff();
    void resetTimer();
    void refreshState();
    void toggle();
};