#include "Ina226Impl.h"

#define INA_226_V_LIMIT 36.0f

Ina226Impl::Ina226Impl(
    const uint8_t address,
    const float rShunt,
    const float maxCurrent) : PowerMonitor(), ina(new INA226(rShunt, maxCurrent, address)) {}

Ina226Impl::~Ina226Impl()
{
    delete ina;
}

bool Ina226Impl::begin(const uint8_t &sdaPin,
                       const uint8_t &sclPin)
{
    auto result = ina->begin(sdaPin, sclPin);
    if (result)
    {
        ina->setAveraging(INA226_AVG_X4);
        ina->setSampleTime(INA226_VBUS, INA226_CONV_1100US);
        ina->setSampleTime(INA226_VSHUNT, INA226_CONV_1100US);
    }
    return result;
}

const float Ina226Impl::maxVoltage()
{
    return INA_226_V_LIMIT;
}

float Ina226Impl::measureCurrent()
{
    return ina->getCurrent();
}

float Ina226Impl::measureVoltage()
{
    return ina->getVoltage();
}

float Ina226Impl::measureShuntVoltage()
{
    return ina->getShuntVoltage();
}

float Ina226Impl::measurePower()
{
    return ina->getPower();
}
