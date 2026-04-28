#pragma once

#include <SettingsAsync.h>

#define BUTTON_APPLY SH("apply")
#define BUTTON_SKIP SH("skip")
#define BUTTON_REBOOT SH("reboot")

#define UNIT_V "V"
#define UNIT_mV "mV"
#define UNIT_PERCENT "%"
#define UNIT_mA "mA"
#define UNIT_A "A"
#define UNIT_mAh "mAh"
#define UNIT_Ah "Ah"
#define UNIT_mWh "mWh"
#define UNIT_Wh "Wh"
#define UNIT_mW "mW"
#define UNIT_W "W"

#define DEVICE_INFO_GROUP "Device info"

DB_KEYS(
    NET_CONFIG,

    wifi_ssid,
    wifi_pass

);

DB_KEYS(
    DEVICE_INFO,

    name,
    os_info,
    serial_number,
    full_capacity,
    connect_status

);

DB_KEYS(
    BARRETY_INFO,

    percent,
    voltage,
    current
);

DB_KEYS(
    TRACKING_VALUES,

    tracking_time,
    totalCurrent,
    totalPower,
    maxCurrent,
    maxPower,
    minVoltage

);

DB_KEYS(
    BAT_MONITOR_CONFIG,

    shunt_resistance,
    max_current,
    measurement_interval,
    ina_device,
    ina_i2c_address,
    power_strategy,
    current_correction,
    voltage_correction

);

DB_KEYS(
    POWER_STRATEGY_CONFIG,

    battery_min_voltage,
    battery_max_voltage,
    power_source_min_voltage,
    power_source_max_voltage

);

DB_KEYS(
    OTA_CONFIG,

    auto_update_enabled,
    check_interval,
    server_url,
    last_version,
    url_params

);

DB_KEYS(
    BOARD_CONFIG,

    i2c_frequency

);

inline size_t withKey(const size_t &baseKey, const uint8_t &address)
{
    return baseKey + address;
}