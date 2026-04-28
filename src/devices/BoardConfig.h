#pragma once

#include <Arduino.h>

enum class BoardType
{
    NODE_MCU,
    HW364A,
    DEFAULT_ARD
};

struct BoardConfig
{
    BoardType type;
    uint8_t sdaPin;
    uint8_t sclPin;

    BoardConfig(const uint8_t &sda = SDA, const uint8_t &scl = SCL, BoardType boardType = BoardType::DEFAULT_ARD)
        : type(boardType), sdaPin(sda), sclPin(scl) {}
};

#if defined(ESP8266)
#define NODEMCU_SDA 12  // GPIO12 (D6)
#define NODEMCU_SCL 14  // GPIO14 (D5)
#define HW364A_SDA  14  // GPIO14 (D5)
#define HW364A_SCL  12  // GPIO12 (D6)
#elif defined(ESP32)
#define NODEMCU_SDA 21  // GPIO21
#define NODEMCU_SCL 22  // GPIO22
#define HW364A_SDA  21
#define HW364A_SCL  22
#else
#define NODEMCU_SDA SDA
#define NODEMCU_SCL SCL
#define HW364A_SDA  SDA
#define HW364A_SCL  SCL
#endif

static const BoardConfig BOARD_CONFIGS[] = {
    {NODEMCU_SDA, NODEMCU_SCL, BoardType::NODE_MCU},
    {HW364A_SDA, HW364A_SCL, BoardType::HW364A},
    {SDA, SCL, BoardType::DEFAULT_ARD}
};