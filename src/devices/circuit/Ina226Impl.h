#include <INA226.h>

#include "api/PowerMonitor.h"

class Ina226Impl : public PowerMonitor
{
public:
    Ina226Impl(
        const uint8_t address = 0x40,
        const float rShunt = 0.005,
        const float maxCurrent = 10);
    ~Ina226Impl();

    bool begin(const uint8_t &sdaPin,
               const uint8_t &sclPin) override;
    const float maxVoltage() override;

    float measureCurrent() override;
    float measureVoltage() override;
    float measureShuntVoltage() override;
    float measurePower() override;

private:
    INA226 *const ina;
};