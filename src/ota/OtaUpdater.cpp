#include "OtaUpdater.h"
#include "Context.h"

#include <GSON.h>

#include "db/data.h"
#include "util/BuildInfo.h"

#include "states/ina/InaStateMachines.h"

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#endif

static size_t indexToSeconds(uint8_t index)
{
    switch (index)
    {
    case 1:
        return 3600; // Every hour
    case 2:
        return 21600; // Every 6 hours
    case 3:
        return 43200; // Every 12 hours
    case 4:
        return 86400; // Every 24 hours
    default:
        return 0; // Never
    }
}

OtaUpdater::OtaUpdater(Context *const context): context(context) {}

void OtaUpdater::begin()
{
    size_t index = context->db->get(OTA_CONFIG::check_interval);
    size_t intervalSec = indexToSeconds(index);
    setCheckInterval(intervalSec * 1000);
    bool isEnabled = context->db->get(OTA_CONFIG::auto_update_enabled);
    setAutoUpgrade(isEnabled);

    lastCheckTime = millis();
    resetUpdateInfo();
}

void OtaUpdater::tick()
{
    if (state == CHECKING)
    {
        return;
    }

    if (forceCheck)
    {
        context->logger->println("[OtaUpd] Force check triggered");
        forceCheck = false;
        lastCheckTime = millis();
        checkForUpdates();
        return;
    }

    if (!context->inaMachines->isAnySensorBusy() && forceInstall && state == UPDATE_AVAILABLE && !context->otaInstaller->isBusy())
    {
        context->logger->println("[OtaUpd] Force install triggered");
        forceInstall = false;
        context->otaInstaller->upgrade(updateInfo.firmwareUrl);
        return;
    }

    if (checkIntervalMs == 0)
    {
        return;
    }

    size_t now = millis();
    //if (now - lastCheckTime >= 30000) // for test imitations
    if (now - lastCheckTime >= checkIntervalMs)
    {
        lastCheckTime = now;
        checkForUpdates();
    }
}

void OtaUpdater::checkForUpdates()
{
    context->logger->println("[OtaUpd] checkForUpdates");
    if (state != IDLE && state != UPDATE_AVAILABLE && state != ERROR)
    {
        context->logger->println("[OtaUpd] Invalid state, skipping");
        return;
    }

    if (!context->wifi->isWifiConnected() || context->inaMachines->isAnySensorBusy() || context->otaInstaller->isBusy())
    {
        context->logger->println("[OtaUpd] skipping");
        return;
    }

    setState(CHECKING);
    errorCode = NONE;

    String serverUrl = context->db->get(OTA_CONFIG::server_url);
    String urlParams = context->db->get(OTA_CONFIG::url_params);
    if (urlParams.length() > 0) {
        serverUrl = addUrlParams(serverUrl, urlParams);
    }
    context->logger->print("[OtaUpd] URL: ");
    context->logger->println(serverUrl);
    if (serverUrl.length() == 0)
    {
        setError(SERVER_URL_NOT_SET);
        setState(ERROR);
        return;
    }

    gson::ParserStream parser;
    if (!fetchFirmwareInfo(serverUrl, parser))
    {
        setState(ERROR);
        return;
    }

    String version = parser["version"];
    uint16_t buildNumber = parser["buildNumber"];

    if (compareVersions(version, buildNumber))
    {
        context->logger->print("[OtaUpd] Upd available: ");
        parser.stringify(*context->logger);
        updateInfo.version = version;
        updateInfo.buildNumber = buildNumber;
        String url = parser[BuildInfo::getPlatform()];
        if (urlParams.length() > 0) {
            url = addUrlParams(url, urlParams);
        }
        updateInfo.firmwareUrl = url;

        setState(UPDATE_AVAILABLE);

        if (autoUpgradeEnabled)
        {
            context->otaInstaller->upgrade(url);
        }
    }
    else
    {
        context->logger->println("[OtaUpd] No update");
        resetUpdateInfo();
        setState(IDLE);
    }
}

const OtaUpdater::UpdateInfo *OtaUpdater::getAvailableInfo() const
{
    if (updateInfo.version.length() == 0)
    {
        return nullptr;
    }
    return &updateInfo;
}

bool OtaUpdater::fetchFirmwareInfo(const String &url, gson::ParserStream &parser)
{
    context->logger->print("[OtaUpd] fetchFirmwareInfo: ");
    context->logger->println(url);
#if defined(ESP8266) || defined(ESP32)
    if (!context->wifi->isWifiConnected())
    {
        context->logger->println("[OtaUpd] WiFi not connected");
        setError(NETWORK_ERROR);
        return false;
    }

    WiFiClient *const selectedClient = context->clientProvider->getClient(url);
    if (selectedClient == nullptr)
    {
        context->logger->println("[OtaUpd] No WifiClient");
        setError(HTTP_ERROR);
        return false;
    }

    // Логируем тип клиента
    bool useSecure = url.startsWith("https://");
    context->logger->print("[OtaUpd] Using ");
    context->logger->println(useSecure ? "WiFiClientSecure" : "WiFiClient");

    HTTPClient http;
    if (!http.begin(*selectedClient, url))
    {
        setError(HTTP_ERROR);
        return false;
    }

    http.setTimeout(3000);
    context->logger->print("[OtaUpd] ");
    context->logger->println(useSecure ? "run HTTPS GET" : "run HTTP GET");
    int httpCode = http.GET();
    context->logger->print("[OtaUpd] HTTP code: ");
    context->logger->println(httpCode);
    if (httpCode != HTTP_CODE_OK)
    {
        setError(HTTP_ERROR);
        http.end();
        return false;
    }

    if (!parser.parse(&http.getStream(), http.getSize()))
    {
        setError(JSON_PARSE_ERROR);
        http.end();
        return false;
    }
    http.end();

    context->logger->println("[OtaUpd] JSON parsed!");
    return true;
#else
    return false;
#endif
}

bool OtaUpdater::compareVersions(const String &serverVersion, const int &serverBuild)
{
    int currentBuild = BuildInfo::getBuildNumber();
    bool result = serverBuild > currentBuild;
    context->logger->print("[OtaUpd] versions: server=");
    context->logger->print(serverBuild);
    context->logger->print(" current=");
    context->logger->print(currentBuild);
    context->logger->print(" result=");
    context->logger->println(result ? "true" : "false");
    return result;
}

void OtaUpdater::resetUpdateInfo()
{
    updateInfo.version = "";
    updateInfo.firmwareUrl = "";
    updateInfo.buildNumber = 0;
}

String OtaUpdater::ensureHttpProtocol(const String &url)
{
    if (url.length() == 0)
    {
        return url;
    }
    if (url.startsWith("http://") || url.startsWith("https://"))
    {
        return url;
    }
    return "http://" + url;
}

String OtaUpdater::addUrlParams(const String &url, const String &params)
{
    if (params.length() == 0)
    {
        return url;
    }
    String result = url;
    if (result.indexOf('?') == -1)
    {
        result += '?';
    }
    else
    {
        result += '&';
    }
    result += params;
    return result;
}

void OtaUpdater::setState(State newState)
{
    static const char *stateNames[] = {"IDLE", "CHECKING", "UPDATE_AVAILABLE", "ERROR"};
    context->logger->print("[OtaUpd] State: ");
    context->logger->print(stateNames[state]);
    context->logger->print(" -> ");
    context->logger->println(stateNames[newState]);
    state = newState;
}

void OtaUpdater::setError(Error code)
{
    errorCode = code;
    context->logger->print("[OtaUpd] Error: ");
    context->logger->println(getErrorString());
}

String OtaUpdater::getErrorString() const
{
    switch (errorCode)
    {
    case NONE:
        return "No error";
    case NETWORK_ERROR:
        return "Network error (WiFi not connected)";
    case HTTP_ERROR:
        return "HTTP error";
    case JSON_PARSE_ERROR:
        return "JSON parse error";
    case SERVER_URL_NOT_SET:
        return "Server URL not set";
    default:
        return "Unknown error";
    }
}
