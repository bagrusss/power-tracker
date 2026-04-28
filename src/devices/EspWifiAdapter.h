#pragma once

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <Print.h>

#include "api/WifiAdapter.h"

class EspWifiAdapter : public WifiAdapter
{
private:
    DisconnectedCallback dcb = nullptr;
    Print *const logger = nullptr;
#if defined(ESP32)
    WiFiEventId_t disconnectHandlerId;
#endif

public:
    EspWifiAdapter(Print *const logger);
    ~EspWifiAdapter();

    bool isWifiConnected() override;
    void connectToAp(const char *ssid, const char *passwd) override;
    bool startAp(const char *ssid, const char *passwd) override;
    void getMacAddress(uint8_t *mac) override;
    String getSsid() const override;
    String localIp() override;
    String apIp() override;
    void setDisconnectedCallback(DisconnectedCallback cb) override;
};