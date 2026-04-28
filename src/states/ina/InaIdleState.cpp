#include "InaIdleState.h"

#include "db/data.h"
#include "util/Util.h"
#include "util/icons.h"
#include "res/Titles.h"
#include "api/PowerMonitor.h"

InaIdleState::InaIdleState(InaStateMachine *const stateMachine, Context *const context, PowerMonitor *const sensor)
    : InaState(stateMachine, context, sensor) {}

void InaIdleState::buildUi(sets::Builder &b)
{
    drawSensorData(b, sensor);

    if (b.Button(UiTitles::UiButtons::TRACK))
    {
        stateMachine->switchState((size_t)InaStateType::Tracking);
        b.reload();
    }
}

void InaIdleState::updateUi(sets::Updater &upd)
{
    updateSensorData(upd, sensor);
}

void InaIdleState::onEnter()
{
}

void InaIdleState::onLeft()
{
}

const char *InaIdleState::getDescription() const
{
    static char description[32];
    snprintf(description, sizeof(description), "INA Idle 0x%02X", stateMachine->getSensorAddress());
    return description;
}

InaStateType InaIdleState::getStateType() const
{
    return InaStateType::Idle;
}

void InaIdleState::drawBattery(sets::Builder &b, Battery *const bt, const float &voltage, const float &current)
{
    auto state = bt->getState(voltage, current);
    sets::Colors currentColor = sets::Colors::Default;
    switch (state.status)
    {
    case Battery::BatteryStatus::FULLY_CHARGED:
        currentColor = sets::Colors::Green;
        break;
    case Battery::BatteryStatus::CHARGING:
        currentColor = sets::Colors::Orange;
        break;
    case Battery::BatteryStatus::DISCHARGING:
        currentColor = sets::Colors::Red;
        break;
    case Battery::BatteryStatus::NO_CURRENT:
        currentColor = sets::Colors::Gray;
        break;
    case Battery::BatteryStatus::DISCHARGED:
        currentColor = sets::Colors::Black;
        break;
    }

    b.LED(DEVICE_INFO::connect_status, UiTitles::DEVICE_STATUS, 1, currentColor, sets::Colors::Red);
    b.LabelNum(BARRETY_INFO::percent, UiTitles::CAPACITY, state.percantage);
}

void InaIdleState::drawPowerSource(sets::Builder &b, PowerSource *const ps, const float &voltage, const float &current)
{
    // Реализация отображения источника питания
}

void InaIdleState::updateBattery(sets::Updater &upd, Battery *const bt, const float &voltage, const float &current)
{
    auto state = bt->getState(voltage, current);
    sets::Colors currentColor = sets::Colors::Default;
    switch (state.status)
    {
    case Battery::BatteryStatus::FULLY_CHARGED:
        currentColor = sets::Colors::Green;
        break;
    case Battery::BatteryStatus::CHARGING:
        currentColor = sets::Colors::Orange;
        break;
    case Battery::BatteryStatus::DISCHARGING:
        currentColor = sets::Colors::Red;
        break;
    case Battery::BatteryStatus::NO_CURRENT:
        currentColor = sets::Colors::Gray;
        break;
    case Battery::BatteryStatus::DISCHARGED:
        currentColor = sets::Colors::Black;
        break;
    }

    upd.updateColor(DEVICE_INFO::connect_status, currentColor);
    upd.update(BARRETY_INFO::percent, state.percantage);
}

void InaIdleState::updatePowerSource(sets::Updater &upd, PowerSource *const ps)
{
    // Реализация обновления источника питания
}

void InaIdleState::drawSensorData(sets::Builder &b, PowerMonitor *const sensor)
{
    if (!sensor)
    {
        b.Label(UiTitles::Sensor::SENSOR_NOT_AVAILABLE);
        return;
    }

    auto current = sensor->getCurrent();
    auto voltage = sensor->getLoadVoltage();
    auto voltageShunt = sensor->getShuntVoltage();
    auto power = sensor->getPower();

    const auto st = sensor->getStrategy();
    if (st)
    {
        switch (st->getType())
        {
        case PowerStrategy::Type::BATTERY:
        {
            drawBattery(b, (Battery *)st, voltage, current);
            break;
        }
        case PowerStrategy::Type::POWER_SOURCE:
        {
            drawPowerSource(b, (PowerSource *)st, voltage, current);
            break;
        }
        }
    }

    b.LabelFloat(0, "Voltage, V", voltage);
    b.LabelFloat(1, "Shunt, mV", voltageShunt);
    b.LabelFloat(2, "Current, mA", current);
    b.LabelFloat(3, "Power, mW", power);
}

void InaIdleState::updateSensorData(sets::Updater &upd, PowerMonitor *const sensor)
{
    const auto voltage = sensor->getLoadVoltage();
    const auto current = sensor->getCurrent();
    const auto st = sensor->getStrategy();
    if (st)
    {
        switch (st->getType())
        {
        case PowerStrategy::Type::BATTERY:
            updateBattery(upd, (Battery *)st, voltage, current);
            break;
        case PowerStrategy::Type::POWER_SOURCE:
            updatePowerSource(upd, (PowerSource *)st);
            break;
        }
    }
    upd.update(0, voltage);
    upd.update(1, sensor->getShuntVoltage());
    upd.update(2, current);
    upd.update(3, sensor->getPower());
}