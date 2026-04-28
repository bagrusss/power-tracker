#pragma once

#include <functional>
#include <WString.h>

class WifiAdapter
{
public:
    typedef void (*DisconnectedCallback)();

    virtual ~WifiAdapter() = default;
    virtual bool isWifiConnected() = 0;
    virtual bool startAp(const char *ssid, const char *passwd = nullptr) = 0;
    virtual void connectToAp(const char *ssid, const char *passwd = nullptr) = 0;
    virtual String getSsid() const = 0;
    virtual String localIp() = 0;
    virtual String apIp() = 0;

    virtual void getMacAddress(uint8_t *mac) = 0;

    virtual void setDisconnectedCallback(DisconnectedCallback cb) = 0;
};