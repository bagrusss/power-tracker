#pragma once

#include "api/PowerStrategy.h"
#include "devices/circuit/InaDetector.h"

class PowerMonitor
{
public:
    PowerMonitor() {}
    
    virtual ~PowerMonitor()
    {
        if (powerStrategy)
        {
            delete powerStrategy;
            powerStrategy = nullptr;
        }
    }

    virtual const float maxVoltage() = 0;

    bool init(const uint8_t &sdaPin,
              const uint8_t &sclPin)
    {
        isReady = begin(sdaPin, sclPin);
        return isReady;
    }

    virtual bool isPrepared()
    {
        return isReady;
    }

    virtual InaType getType() const { return InaType::UNKNOWN; }

    // ток в mA
    float getCurrent(bool isMeasure = true)
    {
        if (isMeasure)
        {
            currentCurrent = measureCurrent();
        }
        return applyCurrentK(currentCurrent) * 1000;
    }

    // напряжение в V
    float getLoadVoltage(bool isMeasure = true)
    {
        if (isMeasure)
        {
            currentVoltage = measureVoltage();
        }
        return applyVoltageK(currentVoltage);
    }

    // напряжение в mV
    float getShuntVoltage(bool isMeasure = true)
    {
        if (isMeasure)
        {
            shuntVoltage = measureShuntVoltage();
        }
        return applyVoltageK(shuntVoltage) * 1000;
    }

    // мощность в mW
    float getPower(bool isMeasure = true)
    {
        if (isMeasure)
        {
            currentPower = measurePower();
        }
        return applyPowerK(currentPower) * 1000;
    }

    bool isActive()
    {
        return getPower() > 25;
    }

    void setStrategy(PowerStrategy *newStrategy)
    {
        if (newStrategy)
        {
            powerStrategy = newStrategy;
        }
    }

    PowerStrategy *const getStrategy()
    {
        return powerStrategy;
    }

    void setCorrections(const float &kC, const float &kV)
    {
        currentK = kC;
        voltageK = kV;
    }

protected:
    float currentVoltage;
    float shuntVoltage;
    float currentPower;
    float currentCurrent;
    float currentK = 0.0f;
    float voltageK = 0.0f;

    bool isReady = false;

    PowerStrategy *powerStrategy = nullptr;

    virtual float measureCurrent() = 0;
    virtual float measureVoltage() = 0;
    virtual float measureShuntVoltage() = 0;
    virtual float measurePower() = 0;

    virtual bool begin(const uint8_t &sdaPin,
                       const uint8_t &sclPin) = 0;

    template <typename T>
    T withValue(const char *name, const T &value) const
    {
        // Serial.println(name + " = " + value);
        return value;
    };

    float applyCurrentK(const float &value) const
    {
        return value * (1.0f + currentK);
    }

    float applyVoltageK(const float &value) const
    {
        return value * (1.0f + voltageK);
    }

    float applyPowerK(const float &value) const
    {
        return value * (1.0f + currentK) * (1.0f + voltageK);
    }
};
