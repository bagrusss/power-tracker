#pragma once

#include <Arduino.h>

// JSON keys — stored in PROGMEM, accessible via JKEY::STATUS, JKEY::TYPE, etc.
// Usage: gs[JKEY::STATUS] = JVAL::ERROR;

namespace JKEY {

#define DEFINE_KEY(name, str) \
    static constexpr char _##name[] PROGMEM = str; \
    const __FlashStringHelper * const name = reinterpret_cast<const __FlashStringHelper*>(_##name)

DEFINE_KEY(STATUS,          "status");
DEFINE_KEY(ERROR,           "error");
DEFINE_KEY(CODE,            "code");
DEFINE_KEY(MESSAGE,         "message");
DEFINE_KEY(RESULT,          "result");
DEFINE_KEY(OPERATION,       "operation");
DEFINE_KEY(SERIAL_NUM,      "serial");
DEFINE_KEY(TYPE,            "type");
DEFINE_KEY(STATE_TYPE,      "stateType");
DEFINE_KEY(TOTAL_TIME,      "totalTime");
DEFINE_KEY(SENSOR_ADDR,     "sensorAddr");
DEFINE_KEY(CURRENT_CURRENT, "currentCurrent");
DEFINE_KEY(TOTAL_CURRENT,   "totalCurrent");
DEFINE_KEY(MAX_CURRENT,     "maxCurrent");
DEFINE_KEY(CURRENT_VOLTAGE, "currentVoltage");
DEFINE_KEY(MIN_VOLTAGE,     "minVoltage");
DEFINE_KEY(CURRENT_POWER,   "currentPower");
DEFINE_KEY(TOTAL_POWER,     "totalPower");
DEFINE_KEY(MAX_POWER,       "maxPower");
DEFINE_KEY(INTERVAL,        "interval");
DEFINE_KEY(SENSORS,         "sensors");
DEFINE_KEY(ADDRESS,         "address");
DEFINE_KEY(STATE,           "state");
DEFINE_KEY(PROGRESS,        "progress");
DEFINE_KEY(FINISHED,        "finished");
DEFINE_KEY(SUCCESS,         "success");

#undef DEFINE_KEY

}  // namespace JKEY

namespace JVAL {

#define DEFINE_VAL(name, str) \
    static constexpr char _##name[] PROGMEM = str; \
    const __FlashStringHelper * const name = reinterpret_cast<const __FlashStringHelper*>(_##name)

DEFINE_VAL(ERROR,        "error");
DEFINE_VAL(TRACKING,     "tracking");
DEFINE_VAL(IDLE,         "idle");
DEFINE_VAL(OTA_UPDATING, "ota_updating");

#undef DEFINE_VAL

}  // namespace JVAL

namespace HTTP {

#define DEFINE_HTTP(name, str) \
    static constexpr char _##name[] PROGMEM = str; \
    const __FlashStringHelper * const name = reinterpret_cast<const __FlashStringHelper*>(_##name)

DEFINE_HTTP(CONTENT_TYPE, "application/json");
DEFINE_HTTP(PARAM_SERIAL, "serial");
DEFINE_HTTP(CMD_START,    "start");
DEFINE_HTTP(CMD_STOP,     "stop");
DEFINE_HTTP(CMD_STATUS,   "status");

#undef DEFINE_HTTP

}  // namespace HTTP
