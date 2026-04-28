#pragma once

class PowerStrategy
{
public:
    enum Type
    {
        BATTERY,
        POWER_SOURCE
    };

    PowerStrategy(const float minV, const float maxV) : minVoltage(minV), maxVoltage(maxV) {}

    virtual Type getType() const = 0;
    virtual ~PowerStrategy() = default;

protected:
    const float minVoltage;
    const float maxVoltage;
};

class Battery final : public PowerStrategy
{
public:
    enum BatteryStatus : uint8_t
    {
        FULLY_CHARGED,
        CHARGING,
        DISCHARGING,
        DISCHARGED,
        NO_CURRENT
    };

    struct BatteryState
    {
        const uint8_t percantage;
        const BatteryStatus status;
    };

    Battery(const float minV, const float maxV) : PowerStrategy(minV, maxV) {}

    PowerStrategy::Type getType() const override
    {
        return PowerStrategy::Type::BATTERY;
    }

    BatteryState getState(const float &currentV, const float &current)
    {
        if (currentV >= maxVoltage)
        {
            return BatteryState{100, BatteryStatus::FULLY_CHARGED};
        }
        else if (currentV <= minVoltage)
        {
            return BatteryState{0, BatteryStatus::DISCHARGED};
        }
        else if (current > 0)
        {
            return BatteryState{getPercent(currentV), BatteryStatus::DISCHARGING};
        }
        else if (current < 0)
        {
            return BatteryState{getPercent(currentV), BatteryStatus::CHARGING};
        }
        else
        {
            return BatteryState{getPercent(currentV), BatteryStatus::NO_CURRENT};
        }
    }

private:
    uint8_t getPercent(const float &currentVoltage)
    {
        return (currentVoltage - minVoltage) / (maxVoltage - minVoltage) * 100;
    }
};

class PowerSource final : public PowerStrategy
{

public:
    PowerSource(const float minV, const float maxV) : PowerStrategy(minV, maxV) {}

    PowerStrategy::Type getType() const override
    {
        return PowerStrategy::Type::POWER_SOURCE;
    }
};