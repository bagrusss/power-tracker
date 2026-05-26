#pragma once

#include <Arduino.h>

namespace UiTitles
{

#define DEFINE_TITLE(name, str) \
    static constexpr char _##name[] PROGMEM = str; \
    const __FlashStringHelper * const name = reinterpret_cast<const __FlashStringHelper*>(_##name)

    DEFINE_TITLE(NO_POWER_MONITOR, "No power monitors found");

    DEFINE_TITLE(CAPACITY, "Capacity, %");
    DEFINE_TITLE(CURRENT, "Current");
    DEFINE_TITLE(VOLTAGE, "Voltage");

    DEFINE_TITLE(TOTAL_CURRENT, "Total current");
    DEFINE_TITLE(TOTAL_ENERGY, "Total energy");
    DEFINE_TITLE(MAX_POWER, "Max power");
    DEFINE_TITLE(MAX_CURRENT, "Max current");
    DEFINE_TITLE(MIN_VOLTAGE, "Min voltage");
    DEFINE_TITLE(TRACKING_TIME, "Tracking time");

    DEFINE_TITLE(SSID, "SSID");
    DEFINE_TITLE(PASSWORD, "Password");

    DEFINE_TITLE(DEVICE_NAME, "Name");
    DEFINE_TITLE(DEVICE_OS, "OS");
    DEFINE_TITLE(DEVICE_SN, "Serial Number");
    DEFINE_TITLE(DEVICE_STATUS, "Status");

    namespace WebUi
    {
        DEFINE_TITLE(TITLE, "Power tracker");
    }

    namespace Groups
    {
        DEFINE_TITLE(DEVICE_INFO, "Device info");
        DEFINE_TITLE(WIFI_SETTINGS, "Wi-Fi settings");
        DEFINE_TITLE(CURCIUT_PARAMS, "Curciut params");
        DEFINE_TITLE(UPDATES, "Updates");
        DEFINE_TITLE(BOARD_CONFIG, "Board Config");
        DEFINE_TITLE(WIFI_CONFIG, "Wi-Fi config");
    }

    namespace UiButtons
    {
        DEFINE_TITLE(STOP, "Stop");
        DEFINE_TITLE(RESTART, "Restart");
        DEFINE_TITLE(BACK, "Back");
        DEFINE_TITLE(SETTINGS, "Settings");
        DEFINE_TITLE(TRACK, "Track");
        DEFINE_TITLE(APPLY, "Apply");
        DEFINE_TITLE(SKIP, "Skip");
        DEFINE_TITLE(APPLY_AND_REBOOT, "Apply and Reboot");
    }

    namespace Messages
    {
        DEFINE_TITLE(SETTED_UP, "setted up!");
        DEFINE_TITLE(CONNECTING_TO_WIFI, "Connecting to WiFi...");
        DEFINE_TITLE(CONNECTING_TO, "Connecting to ");
        DEFINE_TITLE(CONNECTED_TO_WIFI_IP, "Connected to WiFi. IP: ");
        DEFINE_TITLE(FAILED_TO_CONNECT_TO, "Failed to connect to ");
        DEFINE_TITLE(COULDNT_UP_AP, "Couldn't up AP!");
        DEFINE_TITLE(CONNECTING_WIFI, "Connecting Wi-Fi");

        DEFINE_TITLE(INA_NOT_FOUND, "No INA not found");
        DEFINE_TITLE(DEVICE_NOT_FOUND, "Device not found");
        
        DEFINE_TITLE(OTA_INSTALLING, "UPD: installing");
        DEFINE_TITLE(OTA_SUCCESS, "UPD: Success!");
        DEFINE_TITLE(OTA_FAILED, "UPD: Failed!");
    }

    namespace FileNames
    {
        DEFINE_TITLE(DATA_DB, "/data.db");
        DEFINE_TITLE(SETTINGS_PREFIX, "/settings_0x");
    }

    namespace Network
    {
        DEFINE_TITLE(TMP_WIFI_NAME, "PT_");
        DEFINE_TITLE(DEFAULT_WIFI_PASS, "vk_testlab");
        DEFINE_TITLE(AP_PREFIX, "AP: ");
        DEFINE_TITLE(IP_PREFIX, "IP: ");
    }

    namespace Sensor
    {
        DEFINE_TITLE(SENSOR_STATUS, "Sensor Status");
        DEFINE_TITLE(SENSOR_NOT_AVAILABLE, "Sensor not available");
    }

    namespace Settings
    {
        DEFINE_TITLE(FULL_CAPACITY, "Full capacity");
        DEFINE_TITLE(MIN_VOLTAGE_V, "Min Voltage, V");
        DEFINE_TITLE(MAX_VOLTAGE_V, "Max Voltage, V");
        DEFINE_TITLE(POWER_STRATEGY, "Power Strategy");
        DEFINE_TITLE(SHUNT_RESISTANCE_OHM, "Shunt resistance, Ω");
        DEFINE_TITLE(MAX_CURRENT_A, "Max current, A");
        DEFINE_TITLE(MEASUREMENT_INTERVAL_MS, "Measurement interval, ms");
        DEFINE_TITLE(INA_DEVICE, "INA device");
        DEFINE_TITLE(CURRENT_CORRECTION, "Current kFactor");
        DEFINE_TITLE(VOLTAGE_CORRECTION, "Voltage kFactor");
        DEFINE_TITLE(I2C_FREQUENCY, "I2C Frequency, kHz");
    }

    namespace Updates
    {
        DEFINE_TITLE(AUTO_UPDATE, "Auto-update");
        DEFINE_TITLE(CHECK_INTERVAL, "Check interval");
        DEFINE_TITLE(UPDATE_URL, "Update URL");
        DEFINE_TITLE(URL_PARAMS, "URL params");
        DEFINE_TITLE(CURRENT_VERSION_LABEL, "Current version: ");
        DEFINE_TITLE(BUILD_NUMBER_LABEL, "Build number: ");
        DEFINE_TITLE(CHECK_FOR_UPDATES, "Check for updates");
        DEFINE_TITLE(UPGRADE, "Upgrade");
        DEFINE_TITLE(INSTALL_UPDATE_LABEL, "Install update");
        DEFINE_TITLE(INTERVAL_OPTIONS, "Never;Every hour;Every 6 hours;Every 12 hours;Every 24 hours");
    }

    namespace DefaultValues
    {
        // RAM-версии для db->init() и String конкатенации
        static constexpr char DEFAULT_NAME[] = "Undef";
        static constexpr char DEFAULT_NAN[] = "Nan";
        static constexpr char SN_PREFIX[] = "SN_";
    }

#undef DEFINE_TITLE

}
