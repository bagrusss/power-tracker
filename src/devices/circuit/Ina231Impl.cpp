#include "Ina231Impl.h"

#define INA_231_V_LIMIT 28.0f

Ina231Impl::Ina231Impl(
    const uint8_t address,
    const float rShunt,
    const float maxCurrent) : PowerMonitor(), ina(new INA231(rShunt, maxCurrent, address))
{
}

Ina231Impl::~Ina231Impl()
{
    delete ina;
}

bool Ina231Impl::begin(const uint8_t &sdaPin,
                       const uint8_t &sclPin)
{
    auto result = ina->begin(sdaPin, sclPin);
    if (result)
    {
        ina->setAveraging(INA231_AVG_X4);
        ina->setSampleTime(INA231_VBUS, INA231_CONV_1100US);
        ina->setSampleTime(INA231_VSHUNT, INA231_CONV_1100US);
    }
    return result;
}

const float Ina231Impl::maxVoltage()
{
    return INA_231_V_LIMIT;
}

float Ina231Impl::measureCurrent()
{
    return ina->getCurrent();
}

float Ina231Impl::measureVoltage()
{
    return ina->getVoltage();
}

float Ina231Impl::measureShuntVoltage()
{
    return ina->getShuntVoltage();
}

float Ina231Impl::measurePower()
{
    return ina->getPower();
}