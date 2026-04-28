#pragma once

#if defined(ESP8266)
#include <Esp.h>
#elif defined(ESP32)
#include <esp_system.h>
#endif
#include <memory>
#include <Wire.h>

#include "api/Board.h"
#include "BoardConfig.h"

class EspBoard : public Board
{
public:
    EspBoard(BoardConfig *const config, Print *const logger) : cfg(config), logger(logger) {}
    ~EspBoard() { delete cfg; }

    BoardConfig *const getConfig() const override { return cfg; }
    void reboot() override
    {
#if defined(ESP8266)
        ESP.restart();
#elif defined(ESP32)
        esp_restart();
#endif
    };
    uint32_t getFreeHeap() const override
    {
#if defined(ESP8266)
        return ESP.getFreeHeap();
#elif defined(ESP32)
        return esp_get_free_heap_size();
#endif
    }

    void setI2CFreq(const uint32_t &freqHz) override
    {
        Wire.setClock(freqHz);
        logger->print("I2C frequency set to ");
        logger->print(freqHz);
        logger->println(" Hz");
    }

private:
    BoardConfig *const cfg;
    Print *const logger;
};