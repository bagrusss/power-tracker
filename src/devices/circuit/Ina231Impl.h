#include <INA231.h>

#include "api/PowerMonitor.h"

class Ina231Impl : public PowerMonitor
{
public:
    Ina231Impl(
        const uint8_t address = 0x40,
        const float rShunt = 0.005,
        const float maxCurrent = 10);
    ~Ina231Impl();

    bool begin(const uint8_t &sdaPin,
               const uint8_t &sclPin) override;
    const float maxVoltage() override;

    float measureCurrent() override;
    float measureVoltage() override;
    float measureShuntVoltage() override;
    float measurePower() override;

private:
    INA231 *const ina;
};