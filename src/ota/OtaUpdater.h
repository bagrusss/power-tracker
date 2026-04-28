#pragma once

#include <Arduino.h>
#include <GSON.h>
#include <GyverDBFile.h>
#include <WiFiClient.h>

#include "util/BuildInfo.h"
#include "db/data.h"
#include "api/WifiAdapter.h"
#include "OtaInstaller.h"
#include "net/WifiClientProvider.h"


class Context;

class OtaUpdater
{
public:
    enum State
    {
        IDLE,
        CHECKING,
        UPDATE_AVAILABLE,
        ERROR
    };

    enum Error
    {
        NONE,
        NETWORK_ERROR,
        HTTP_ERROR,
        JSON_PARSE_ERROR,
        SERVER_URL_NOT_SET,
    };

    struct UpdateInfo
    {
        String version;
        String firmwareUrl;
        int buildNumber = 0;
    };

    OtaUpdater(Context *const context);

    void begin();
    void tick();

    void checkForUpdates();
    const UpdateInfo *getAvailableInfo() const;
    void setCheckInterval(const size_t &intervalMs)
    {
        this->checkIntervalMs = intervalMs;
    }
    void setAutoUpgrade(const bool &isEnabled)
    {
        this->autoUpgradeEnabled = isEnabled;
    }
    void setForceCheck(bool force)
    {
        this->forceCheck = force;
    }
    void setForceInstall(bool force)
    {
        this->forceInstall = force;
    }

    State getState() const { return state; }
    Error getError() const { return errorCode; }
    String getErrorString() const;

private:
    Context *const context;

    State state = IDLE;
    Error errorCode = NONE;
    ulong lastCheckTime = 0;
    size_t checkIntervalMs = 0;
    bool autoUpgradeEnabled = false;
    bool forceCheck = false;
    bool forceInstall = false;

    UpdateInfo updateInfo;

    String ensureHttpProtocol(const String &url);
    String addUrlParams(const String &url, const String &params);

    bool fetchFirmwareInfo(const String &url, gson::ParserStream &parser);
    bool compareVersions(const String &serverVersion, const int &serverBuild);
    void resetUpdateInfo();

    void setState(State newState);
    void setError(Error code);
};