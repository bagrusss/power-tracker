#pragma once

namespace UiTitles
{

    constexpr const char *const NO_POWER_MONITOR = "No power monitors found";

    constexpr const char *const CAPACITY = "Capacity, %";
    constexpr const char *const CURRENT = "Current";
    constexpr const char *const VOLTAGE = "Voltage";
    constexpr const char *const VOLTAGE_SHUNT = "Voltage shunt";
    constexpr const char *const POWER = "Power";

    constexpr const char *const TOTAL_CURRENT = "Total current";
    constexpr const char *const TOTAL_ENERGY = "Total energy";
    constexpr const char *const MAX_POWER = "Max power";
    constexpr const char *const MAX_CURRENT = "Max current";
    constexpr const char *const MIN_VOLTAGE = "Min voltage";
    constexpr const char *const TRACKING_TIME = "Tracking time";

    constexpr const char *const SSID = "SSID";
    constexpr const char *const PASSWORD = "Password";

    constexpr const char *const DEVICE_NAME = "Name";
    constexpr const char *const DEVICE_OS = "OS";
    constexpr const char *const DEVICE_SN = "Serial Number";
    constexpr const char *const DEVICE_STATUS = "Status";

    namespace WebUi
    {
        constexpr const char *const TITLE = "Power tracker";
    }

    namespace Groups
    {
        constexpr const char *const MEASURMENT = "Measurment";
        constexpr const char *const DEVICE_INFO = "Device info";
        constexpr const char *const WIFI_SETTINGS = "Wi-Fi settings";
        constexpr const char *const CURCIUT_PARAMS = "Curciut params";
        constexpr const char *const UPDATES = "Updates";
        constexpr const char *const BOARD_CONFIG = "Board Config";

        constexpr const char *const POWER_STATE = "Power state";
        constexpr const char *const WIFI_CONFIG = "Wi-Fi config";
    }

    namespace UiButtons
    {
        constexpr const char *const STOP = "Stop";
        constexpr const char *const RESTART = "Restart";
        constexpr const char *const BACK = "Back";
        constexpr const char *const SETTINGS = "Settings";
        constexpr const char *const TRACK = "Track";
        constexpr const char *const APPLY = "Apply";
        constexpr const char *const SKIP = "Skip";
        constexpr const char *const APPLY_AND_REBOOT = "Apply and Reboot";
    }

    namespace Messages
    {
        constexpr const char *const SETTED_UP = "setted up!";
        constexpr const char *const CONNECTING_TO_WIFI = "Connecting to WiFi...";
        constexpr const char *const CONNECTING_TO = "Connecting to ";
        constexpr const char *const CONNECTED_TO_WIFI_IP = "Connected to WiFi. IP: ";
        constexpr const char *const FAILED_TO_CONNECT_TO = "Failed to connect to ";
        constexpr const char *const COULDNT_UP_AP = "Couldn't up AP!";
        constexpr const char *const CONNECTING_WIFI = "Connecting Wi-Fi";

        constexpr const char *const INA_NOT_FOUND = "No INA not found";
        constexpr const char *const DEVICE_NOT_FOUND = "Device not found";
        
        constexpr const char *const OTA_INSTALLING = "UPD: installing";
        constexpr const char *const OTA_SUCCESS = "UPD: Success!";
        constexpr const char *const OTA_FAILED = "UPD: Failed!";
    }

    namespace FileNames
    {
        constexpr const char *const DATA_DB = "/data.db";
        constexpr const char *const RESULTS_JSON = "/results.json";
        constexpr const char *const RESULTS_SENSOR_PREFIX = "/results_sensor_";
        constexpr const char *const SETTINGS_PREFIX = "/settings_0x";
    }

    namespace Network
    {
        constexpr const char *const TMP_WIFI_NAME = "PT_";
        constexpr const char *const DEFAULT_WIFI_PASS = "power_tracker";
        constexpr const char *const AP_PREFIX = "AP: ";
        constexpr const char *const IP_PREFIX = "IP: ";
    }

    namespace Sensor
    {
        constexpr const char *const SENSOR_PREFIX = "Sensor 0x";
        constexpr const char *const TRACKING_SENSOR_PREFIX = "Tracking Sensor 0x";
        constexpr const char *const SENSOR_STATUS = "Sensor Status";
        constexpr const char *const SENSOR_NOT_AVAILABLE = "Sensor not available";
        constexpr const char *const RESULTS_SAVED_TO = "Results saved to ";
        constexpr const char *const RESULTS_WERE_SAVED = "The results were saved into file ";
    }

    namespace Settings
    {
        constexpr const char *const FULL_CAPACITY = "Full capacity";
        constexpr const char *const MIN_VOLTAGE_V = "Min Voltage, V";
        constexpr const char *const MAX_VOLTAGE_V = "Max Voltage, V";
        constexpr const char *const POWER_STRATEGY = "Power Strategy";
        constexpr const char *const BATTERY = "Battery";
        constexpr const char *const POWER_SOURCE = "Power Source";
        constexpr const char *const SHUNT_RESISTANCE_OHM = "Shunt resistance, Ω";
        constexpr const char *const MAX_CURRENT_A = "Max current, A";
        constexpr const char *const MEASUREMENT_INTERVAL_MS = "Measurement interval, ms";
        constexpr const char *const INA_DEVICE = "INA device";
        constexpr const char *const INA_I2C_ADDRESS = "INA I2C address";
        constexpr const char *const CURRENT_CORRECTION = "Current kFactor";
        constexpr const char *const VOLTAGE_CORRECTION = "Voltage kFactor";
        constexpr const char *const I2C_FREQUENCY = "I2C Frequency, kHz";
    }

    namespace Updates
    {
        constexpr const char *const AUTO_UPDATE = "Auto-update";
        constexpr const char *const CHECK_INTERVAL = "Check interval";
        constexpr const char *const UPDATE_URL = "Update URL";
        constexpr const char *const URL_PARAMS = "URL params";
        constexpr const char *const CURRENT_VERSION_LABEL = "Current version: ";
        constexpr const char *const AVAILABLE_VERSION_LABEL = "Available version: ";
        constexpr const char *const BUILD_NUMBER_LABEL = "Build number: ";
        constexpr const char *const CHECK_FOR_UPDATES = "Check for updates";
        constexpr const char *const INSTALL = "Install";
        constexpr const char *const UPGRADE = "Upgrade";
        constexpr const char *const INSTALL_UPDATE_LABEL = "Install update";
        constexpr const char *const INTERVAL_OPTIONS = "Never;Every hour;Every 6 hours;Every 12 hours;Every 24 hours";
    }

    namespace DefaultValues
    {
        constexpr const char *const DEFAULT_NAME = "Undef";
        constexpr const char *const DEFAULT_NAN = "Nan";
        constexpr const char *const SN_PREFIX = "SN_";
    }

}