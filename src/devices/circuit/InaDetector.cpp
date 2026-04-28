#include "InaDetector.h"

#include "Ina219Impl.h"
#include "Ina226Impl.h"
#include "Ina231Impl.h"
#include "db/data.h"
#include "res/Titles.h"
#include "api/PowerStrategy.h"

#define INA_CONFIG_REG 0x00
#define INA_MANUFACTURER_ID 0xFE
#define INA_DIE_ID 0xFF

static PowerStrategy *createPowerStrategyForAddress(GyverDBFile *const db, const uint8_t &address)
{
    uint8_t strategyType = db->get(withKey(BAT_MONITOR_CONFIG::power_strategy, address));
    float minVoltage = 0, maxVoltage = 0;
    if (strategyType == PowerStrategy::Type::BATTERY)
    {   
        minVoltage = db->get(withKey(POWER_STRATEGY_CONFIG::battery_min_voltage, address));
        maxVoltage = db->get(withKey(POWER_STRATEGY_CONFIG::battery_max_voltage, address));
        return new Battery(minVoltage, maxVoltage);
    }
    else
    {
        minVoltage = db->get(withKey(POWER_STRATEGY_CONFIG::power_source_min_voltage, address));
        maxVoltage = db->get(withKey(POWER_STRATEGY_CONFIG::power_source_max_voltage, address));
        return new PowerSource(minVoltage, maxVoltage);
    }
}

InaDetector::InaDetector(Print *const logger,
                         BoardConfig *const config) : logger(logger), config(config) {}

// https://www.ti.com/lit/ds/symlink/ina231.pdf?ts=1756310463661
// https://www.ti.com/lit/ds/symlink/ina226.pdf?ts=1756258053960
// 231 обнаруживается как 226й. Возможно стоит вручную назначать тип при первичной настройке через веб морду ¯\_(ツ)_/¯
const InaType InaDetector::detectType(const uint8_t &i2cAddress) const
{
    logger->print("INA addr = 0x");
    logger->println(i2cAddress, HEX);

    if (!isDevicePresent(i2cAddress))
    {
        logger->println("not found");
        return InaType::UNKNOWN;
    }

    const uint16_t manufacturerId = readRegister(i2cAddress, INA_MANUFACTURER_ID);
    logger->print("Man ID: 0x");
    logger->println(manufacturerId, HEX);

    if (manufacturerId == 0x5449)
    {
        const uint16_t dieId = readRegister(i2cAddress, INA_DIE_ID);
        logger->print("Die ID: 0x");
        logger->println(dieId, HEX);

        switch (dieId)
        {
        case 0x2260:
            return InaType::INA226;
        case 0x2310:
            return InaType::INA231;
        default:
            logger->print("Unknown device with Die ID: 0x");
            logger->println(dieId, HEX);
            return InaType::UNKNOWN;
        }
    }
    else
    {
        // Может быть INA219 или другое устройство ¯\_(ツ)_/¯
        const uint16_t configReg = readRegister(i2cAddress, INA_CONFIG_REG);
        if (configReg != 0xFFFF && configReg != 0x0000)
        {
            // Эвристика: если регистр конфигурации читается и не в состоянии по умолчанию,
            // возможно, это INA219. Но это не точно.
            logger->print("Non-TI device, config register: 0x");
            logger->println(configReg, HEX);
            return InaType::INA219;
        }
    }

    return InaType::UNKNOWN;
}

bool InaDetector::isDevicePresent(const uint8_t &i2cAddress)
{
    Wire.beginTransmission(i2cAddress);
    return (bool)!Wire.endTransmission();
}

uint16_t InaDetector::readRegister(const uint8_t &i2cAddress, const uint8_t &reg)
{
    Wire.beginTransmission(i2cAddress);
    Wire.write(reg);
    Wire.endTransmission();

    if (Wire.requestFrom(i2cAddress, (uint8_t)2) == 2)
    {
        return (Wire.read() << 8) | Wire.read();
    }
    return 0xFFFF;
}

PowerMonitor *InaDetector::createInaMonitor(const InaType &type, const uint8_t &address, GyverDBFile *const db) const
{
    const float shuntResistance = db->get(withKey(BAT_MONITOR_CONFIG::shunt_resistance, address));
    const float maxCurrent = db->get(withKey(BAT_MONITOR_CONFIG::max_current, address));

    switch (type)
    {
    case InaType::INA219:
        return new Ina219Impl(address, shuntResistance, maxCurrent);
    case InaType::INA226:
        return new Ina226Impl(address, shuntResistance, maxCurrent);
    case InaType::INA231:
        return new Ina231Impl(address, shuntResistance, maxCurrent);
    default:
        return nullptr;
    }
}

std::map<uint8_t, std::unique_ptr<PowerMonitor>> *InaDetector::detectDevices(const uint8_t &startAddr, const uint8_t &endAddr, GyverDBFile *const db) const
{
    std::map<uint8_t, std::unique_ptr<PowerMonitor>> *devices = new std::map<uint8_t, std::unique_ptr<PowerMonitor>>();

    const int8_t step = startAddr <= endAddr ? 1 : -1;
    InaType inaType = InaType::UNKNOWN;
    for (uint8_t addr = startAddr; step > 0 ? addr <= endAddr : addr >= endAddr; addr += step)
    {
        inaType = detectType(addr);
        if (inaType != InaType::UNKNOWN)
        {
            initDeviceSettings(addr, db, inaType);
            PowerMonitor *const monitor = createInaMonitor(inaType, addr, db);
            if (monitor)
            {
                float kC = db->get(withKey(BAT_MONITOR_CONFIG::current_correction, addr));
                float kV = db->get(withKey(BAT_MONITOR_CONFIG::voltage_correction, addr));
                monitor->setCorrections(kC, kV);

                PowerStrategy *strategy = createPowerStrategyForAddress(db, addr);
                monitor->setStrategy(strategy);
                devices->emplace(addr, monitor);
                this->logger->print("Created monitor on addr 0x");
                this->logger->print(addr, HEX);
                this->logger->print(" init=");
                this->logger->println(monitor->init(config->sdaPin, config->sclPin));
            }
        }
    }
    return devices;
}

void InaDetector::initDeviceSettings(const uint8_t &address, GyverDBFile *const db, const InaType &inaType) const
{
    db->init(withKey(BAT_MONITOR_CONFIG::ina_device, address), (uint8_t)inaType);
    db->init(withKey(BAT_MONITOR_CONFIG::shunt_resistance, address), 0.1f);
    db->init(withKey(BAT_MONITOR_CONFIG::max_current, address), 3.2f);
    db->init(withKey(BAT_MONITOR_CONFIG::current_correction, address), 0.0f);
    db->init(withKey(BAT_MONITOR_CONFIG::voltage_correction, address), 0.0f);
    db->init(withKey(BAT_MONITOR_CONFIG::measurement_interval, address), 500);
    db->init(withKey(BAT_MONITOR_CONFIG::power_strategy, address), (uint8_t)PowerStrategy::Type::BATTERY);

    db->init(withKey(POWER_STRATEGY_CONFIG::battery_min_voltage, address), 3.3);
    db->init(withKey(POWER_STRATEGY_CONFIG::battery_max_voltage, address), 4.2);
    db->init(withKey(POWER_STRATEGY_CONFIG::power_source_min_voltage, address), 3.5);
    db->init(withKey(POWER_STRATEGY_CONFIG::power_source_max_voltage, address), 4.0);

    String defaultSerialNumber = UiTitles::DefaultValues::SN_PREFIX + String(address, HEX);
    db->init(withKey(DEVICE_INFO::serial_number, address), defaultSerialNumber);
    db->init(withKey(DEVICE_INFO::name, address), UiTitles::DefaultValues::DEFAULT_NAME);
    db->init(withKey(DEVICE_INFO::os_info, address), UiTitles::DefaultValues::DEFAULT_NAME);
    db->init(withKey(DEVICE_INFO::full_capacity, address), UiTitles::DefaultValues::DEFAULT_NAN);

    this->logger->print("Init settings for 0x");
    this->logger->print(address, HEX);
    this->logger->print(" type=");
    this->logger->println((uint8_t)inaType);
}
