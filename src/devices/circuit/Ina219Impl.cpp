#include "Ina219Impl.h"

#define INA_219_V_LIMIT 26.0f

Ina219Impl::Ina219Impl(
    const uint8_t address,
    const float rShunt,
    const float maxCurrent) : PowerMonitor(), ina(new INA219(rShunt, maxCurrent, address))
{
}

Ina219Impl::~Ina219Impl()
{
    delete ina;
}

bool Ina219Impl::begin(const uint8_t &sdaPin,
                       const uint8_t &sclPin)
{
    auto result = ina->begin(sdaPin, sclPin);
    if (result)
    {
        ina->setResolution(INA219_VBUS, INA219_RES_12BIT_X4);
        ina->setResolution(INA219_VSHUNT, INA219_RES_12BIT_X4);
    }
    return result;
}

const float Ina219Impl::maxVoltage()
{
    return INA_219_V_LIMIT;
}

float Ina219Impl::measureCurrent()
{
    return ina->getCurrent();
}

float Ina219Impl::measureVoltage()
{
    return ina->getVoltage();
}

float Ina219Impl::measureShuntVoltage()
{
    return ina->getShuntVoltage();
}

float Ina219Impl::measurePower()
{
    return ina->getPower();
}
