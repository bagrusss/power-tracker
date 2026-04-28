#include "api/LedIndicator.h"

LedIndicator::LedIndicator(int pin, bool activeLow) : pin(pin), activeLow(activeLow)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
}

void LedIndicator::setBlinkDurations(uint16_t onDuration, uint16_t offDuration)
{
    this->onDuration = max(onDuration, (uint16_t)20);
    this->offDuration = max(offDuration, (uint16_t)20);
    resetTimer();
}

void LedIndicator::setEnabled(bool enable)
{
    if (isEnabled != enable)
    {
        isEnabled = enable;
        isConstantOn = false;
        resetTimer();
        refreshState();
    }
}

void LedIndicator::setConstantOn(bool enable)
{
    if (isConstantOn != enable)
    {
        isConstantOn = enable;
        isEnabled = false;
        refreshState();
    }
}

void LedIndicator::update()
{
    if (!isEnabled || isConstantOn)
        return;

    if (millis() - lastBlinkTime >= (ledState ? onDuration : offDuration))
    {
        toggle();
        resetTimer();
    }
}
void LedIndicator::setOn()
{
    digitalWrite(pin, activeLow ? LOW : HIGH);
}

void LedIndicator::setOff()
{
    digitalWrite(pin, activeLow ? HIGH : LOW);
}

void LedIndicator::resetTimer()
{
    lastBlinkTime = millis();
}

void LedIndicator::refreshState()
{
    if (isConstantOn)
    {
        setOn();
    }
    else if (isEnabled)
    {
        if (ledState)
            setOn();
        else
            setOff();
    }
    else
    {
        setOff();
    }
}

void LedIndicator::toggle()
{
    ledState = !ledState;
    if (ledState)
    {
        setOn();
    }
    else
    {
        setOff();
    }
}
