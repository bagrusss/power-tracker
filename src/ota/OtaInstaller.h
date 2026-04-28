#pragma once

#include <Arduino.h>
#include <WiFiClient.h>

#include "net/WifiClientProvider.h"

// Platform-specific includes
#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#define HTTP_UPDATE ESPhttpUpdate
#elif defined(ESP32)
#include <HTTPClient.h>
#include <Update.h>
#define HTTP_UPDATE HTTPUpdate
#else
#error "Unsupported platform for OTA"
#endif

class Context;
class StateSwitcher;

class OtaUpdaterCallback
{
public:
    virtual void onUpgradeStart() = 0;
    virtual void onUpgradeProgress(const int progress) = 0;
    virtual void onUpgradeEnd(const bool success) = 0;
};

class OtaInstaller
{
public:
    enum State
    {
        IDLE,
        DOWNLOADING,
        INSTALLING,
        ERROR
    };

    enum Error
    {
        NONE,
        NETWORK_ERROR,
        UPDATE_FAILED,
        PLATFORM_UNSUPPORTED
    };

    OtaInstaller(Context *const context);
    ~OtaInstaller();

    bool upgrade(const String &url);

    int getProgress() const { return progress; }

    State getState() const { return state; }

    bool isBusy() const { return state == DOWNLOADING || state == INSTALLING; }

    Error getErrorCode() const { return errorCode; }

    String getError() const;

    void reset();

    void setUpdaterCallback(OtaUpdaterCallback *cb) { updaterCallback = cb; }

    void setStateSwitcher(StateSwitcher *const switcher);

private:
    Context *const context;
    State state = IDLE;
    Error errorCode = NONE;
    int progress = 0;

    OtaUpdaterCallback *updaterCallback = nullptr;
    StateSwitcher *stateSwitcher = nullptr;

    void setState(State newState);
    void setError(Error code);
};