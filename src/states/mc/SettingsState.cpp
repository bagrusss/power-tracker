#include "SettingsState.h"

#include <LittleFS.h>

#include "db/data.h"
#include "res/Titles.h"
#include "util/Prefs.h"
#include "util/BuildInfo.h"
#include "devices/circuit/InaDetector.h"
#include "api/PowerStrategy.h"
#include "ota/OtaUpdater.h"

static size_t indexToSeconds(const uint8_t &index)
{
    switch (index)
    {
    case 1:
        return 3600;
    case 2:
        return 21600;
    case 3:
        return 43200;
    case 4:
        return 86400;
    default:
        return 0;
    }
}

SettingsState::SettingsState(
    StateSwitcher *const stateMachine,
    Context *const context) : State(stateMachine, context) {}

void SettingsState::buildUi(sets::Builder &b)
{
    if (b.beginMenu(UiTitles::Groups::WIFI_SETTINGS))
    {
        if (b.Input(NET_CONFIG::wifi_ssid, UiTitles::SSID) || b.Pass(NET_CONFIG::wifi_pass, UiTitles::PASSWORD))
        {
            wifiChanged = true;
        }

        b.endMenu();
    }

    if (b.beginMenu(UiTitles::Groups::BOARD_CONFIG))
    {
        if (b.Spinner(BOARD_CONFIG::i2c_frequency, UiTitles::Settings::I2C_FREQUENCY, 100, 400, 100))
        {
            curcuitChanged = true;
        }

        b.endMenu();
    }

    if (b.beginMenu(UiTitles::Groups::UPDATES))
    {
        b.Pass(OTA_CONFIG::server_url, UiTitles::Updates::UPDATE_URL);
        b.Pass(OTA_CONFIG::url_params, UiTitles::Updates::URL_PARAMS);

        if (b.Switch(OTA_CONFIG::auto_update_enabled, UiTitles::Updates::AUTO_UPDATE))
        {
            uint8_t isEnabled = b.build.value;
            context->otaUpdater->setAutoUpgrade(isEnabled);
        }

        auto ota = context->otaUpdater;

        if (b.Select(OTA_CONFIG::check_interval, UiTitles::Updates::CHECK_INTERVAL, UiTitles::Updates::INTERVAL_OPTIONS))
        {
            uint8_t index = b.build.value;
            size_t intervalSec = indexToSeconds(index);
            ota->setCheckInterval(intervalSec * 1000);
        }

        b.Label(UiTitles::Updates::CURRENT_VERSION_LABEL, BuildInfo::getVersion(), sets::Colors::Green);
        b.Label(UiTitles::Updates::BUILD_NUMBER_LABEL, String(BuildInfo::getBuildNumber()), sets::Colors::Gray);

        if (ota->getState() == OtaUpdater::CHECKING)
        {
            b.Label("Status", "Checking for updates...", sets::Colors::Default);
        }

        const auto *info = ota->getAvailableInfo();
        if (info && info->buildNumber > BuildInfo::getBuildNumber())
        {
            b.Label("New version available", info->version, sets::Colors::Green);
        }

        if (b.beginButtons())
        {
            if (info && info->version.length() > 0)
            {
                if (b.Button(UiTitles::Updates::UPGRADE))
                {
                    ota->setForceInstall(true);
                    b.reload();
                }
            }
            else
            {
                if (b.Button(UiTitles::Updates::CHECK_FOR_UPDATES))
                {
                    ota->setForceCheck(true);
                    b.reload();
                }
            }

            b.endButtons();
        }

        b.endMenu();
    }

    if (!context->monitors->empty())
    {
        if (b.Tabs(getSensorTabLabels(), &activeTab))
        {
            b.reload();
            return;
        }

        uint8_t currentAddr = getSensorAddressForTab(activeTab);

        if (b.beginGroup(UiTitles::Groups::DEVICE_INFO))
        {
            b.Input(withKey(DEVICE_INFO::name, currentAddr), UiTitles::DEVICE_NAME);
            b.Input(withKey(DEVICE_INFO::os_info, currentAddr), UiTitles::DEVICE_OS);
            b.Input(withKey(DEVICE_INFO::serial_number, currentAddr), UiTitles::DEVICE_SN);
            b.Number(withKey(DEVICE_INFO::full_capacity, currentAddr), UiTitles::Settings::FULL_CAPACITY);

            b.endGroup();
        }

        if (b.beginGroup(UiTitles::Groups::CURCIUT_PARAMS))
        {
            if (b.Select(withKey(BAT_MONITOR_CONFIG::power_strategy, currentAddr), UiTitles::Settings::POWER_STRATEGY, Prefs::POWER_STRATEGIES))
            {
                curcuitChanged = true;
                b.reload();
                return;
            }

            uint8_t currentStrategy = context->db->get(withKey(BAT_MONITOR_CONFIG::power_strategy, currentAddr));
            if (currentStrategy == PowerStrategy::Type::BATTERY)
            {
                if (b.Number(withKey(POWER_STRATEGY_CONFIG::battery_min_voltage, currentAddr), UiTitles::Settings::MIN_VOLTAGE_V))
                {
                    curcuitChanged = true;
                }
                if (b.Number(withKey(POWER_STRATEGY_CONFIG::battery_max_voltage, currentAddr), UiTitles::Settings::MAX_VOLTAGE_V))
                {
                    curcuitChanged = true;
                }
            }
            else
            {
                if (b.Number(withKey(POWER_STRATEGY_CONFIG::power_source_min_voltage, currentAddr), UiTitles::Settings::MIN_VOLTAGE_V))
                {
                    curcuitChanged = true;
                };
                if (b.Number(withKey(POWER_STRATEGY_CONFIG::power_source_max_voltage, currentAddr), UiTitles::Settings::MAX_VOLTAGE_V))
                {
                    curcuitChanged = true;
                };
            }

            if (b.Number(withKey(BAT_MONITOR_CONFIG::shunt_resistance, currentAddr), UiTitles::Settings::SHUNT_RESISTANCE_OHM))
            {
                curcuitChanged = true;
            };
            if (b.Number(withKey(BAT_MONITOR_CONFIG::max_current, currentAddr), UiTitles::Settings::MAX_CURRENT_A))
            {
                curcuitChanged = true;
            };
            if (b.Number(withKey(BAT_MONITOR_CONFIG::current_correction, currentAddr), UiTitles::Settings::CURRENT_CORRECTION))
            {
                curcuitChanged = true;
            };
            if (b.Number(withKey(BAT_MONITOR_CONFIG::voltage_correction, currentAddr), UiTitles::Settings::VOLTAGE_CORRECTION))
            {
                curcuitChanged = true;
            };

            b.Number(withKey(BAT_MONITOR_CONFIG::measurement_interval, currentAddr), UiTitles::Settings::MEASUREMENT_INTERVAL_MS);

            if (b.Select(withKey(BAT_MONITOR_CONFIG::ina_device, currentAddr), UiTitles::Settings::INA_DEVICE, Prefs::INA_DEVICES))
            {
                curcuitChanged = true;
            };

            b.endGroup();
        }
    }

    if (b.Button(BUTTON_APPLY, UiTitles::UiButtons::APPLY))
    {
        context->db->update();
        stateMachine->switchState(StateType::Idle);
        b.reload();
    }
}

void SettingsState::onEnter()
{
    wifiChanged = curcuitChanged = false;
    context->ledIndicator->setConstantOn(1);
}

void SettingsState::onLeft()
{
    context->ledIndicator->setConstantOn(0);
    if (wifiChanged || curcuitChanged)
    {
        context->board->reboot();
    }
}

const char *SettingsState::getDescription() const
{
    return "Settings";
}
