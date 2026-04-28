#include "IdleState.h"

#include "db/data.h"
#include "util/Util.h"
#include "util/icons.h"
#include "res/Titles.h"
#include "states/mc/McStateMachine.h"
#include "states/ina/InaStateMachine.h"
#include "states/ina/InaStateMachines.h"
#include "devices/circuit/InaDetector.h"

static const char *inaTypeToString(const InaType &type)
{
    switch (type)
    {
    case InaType::INA219:
        return "INA219";
    case InaType::INA226:
        return "INA226";
    case InaType::INA231:
        return "INA231";
    default:
        return "UNKNOWN";
    }
}

#define CONFIRM_START 102
#define TAB_NAVIGATION_START 200

IdleState::IdleState(StateSwitcher *const stateMachine, Context *const context)
    : State(stateMachine, context) {}

void IdleState::updateActiveInaMachine()
{
    uint8_t activeAddress = getSensorAddressForTab(activeTab);
    activeInaMachine = context->inaMachines->getInaStateMachine(activeAddress);
}

void IdleState::buildUi(sets::Builder &b)
{
    if (context->monitors->empty())
    {
        b.Label(UiTitles::NO_POWER_MONITOR);
    }
    else
    {
        uint8_t activeAddress = getSensorAddressForTab(activeTab);

        auto lastTab = activeTab;
        if (b.Tabs(getSensorTabLabels(), &activeTab) && lastTab != activeTab)
        {
            updateActiveDevice();
            updateActiveInaMachine();

            b.reload();
            return;
        };

        if (activeDevice)
        {
            if (b.beginGroup(UiTitles::Groups::DEVICE_INFO))
            {
                b.Label(withKey(DEVICE_INFO::name, activeAddress), UiTitles::DEVICE_NAME);
                b.Label(withKey(DEVICE_INFO::os_info, activeAddress), UiTitles::DEVICE_OS);
                b.Label(withKey(DEVICE_INFO::serial_number, activeAddress), UiTitles::DEVICE_SN);

                uint8_t inaType = context->db->get(withKey(BAT_MONITOR_CONFIG::ina_device, activeAddress));
                b.Label(withKey(BAT_MONITOR_CONFIG::ina_device, activeAddress), UiTitles::Settings::INA_DEVICE, inaTypeToString((InaType)inaType));

                b.endGroup();
            }

            if (b.beginGroup(UiTitles::Sensor::SENSOR_STATUS))
            {
                if (activeInaMachine)
                {
                    activeInaMachine->buildUI(b);
                }
                else
                {
                    b.Label(UiTitles::NO_POWER_MONITOR);
                }

                b.endGroup();
            }
        }

        if (b.Button(UiTitles::UiButtons::SETTINGS))
        {
            stateMachine->switchState(StateType::Settings);
            b.reload();
        }
    }
}

void IdleState::updateUi(sets::Updater &upd)
{
    if (activeDevice)
    {
        upd.update(DEVICE_INFO::connect_status, activeDevice->isActive());
    }

    if (confirmStart)
    {
        confirmStart = false;
        upd.update(CONFIRM_START, true);
    }

    if (activeInaMachine)
    {
        activeInaMachine->updateUi(upd);
    }
}

void IdleState::onEnter()
{
    updateActiveDevice();
    updateActiveInaMachine();

    context->ledIndicator->setEnabled(true);
    context->ledIndicator->setBlinkDurations(500, 1500);
    ledBlinkFast = false;

    auto active = checkAndprintSensorAddresses();
    updateLedBlink(active);
}

void IdleState::onLeft()
{
    context->ledIndicator->setEnabled(false);
}

const char *IdleState::getDescription() const
{
    return "Idle";
}

void IdleState::updateBattery(sets::Updater &upd, Battery *const bt)
{
}

void IdleState::updatePowerSource(sets::Updater &upd, PowerSource *const ps)
{
}

void IdleState::updateLedBlink(const uint8_t &activeSensors)
{
    if (activeSensors > 0 && !ledBlinkFast)
    {
        context->ledIndicator->setBlinkDurations(300, 300);
        ledBlinkFast = true;
    }
    else if (!activeSensors && ledBlinkFast)
    {
        context->ledIndicator->setBlinkDurations(500, 1500);
        ledBlinkFast = false;
    }
}

void IdleState::handle()
{
    if (activeInaMachine)
    {
        activeInaMachine->handle();
    }
}

void IdleState::onSensorStatusChanged(const uint8_t address, const bool isBusy)
{
    auto active = checkAndprintSensorAddresses();
    updateLedBlink(active);
}
