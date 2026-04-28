#include "OtaInstaller.h"
#include "states/mc/State.h"
#include "Context.h"

OtaInstaller::OtaInstaller(Context *const context) : context(context) {}

OtaInstaller::~OtaInstaller()
{
}

bool OtaInstaller::upgrade(const String &url)
{
    if (state != IDLE && state != ERROR)
    {
        context->logger->println("[OtaInst] Cannot upgrade: state not IDLE/ERROR");
        return false;
    }

    const size_t freeHeap = ESP.getFreeHeap();
    const size_t requiredHeap = 10240; // 10 KB минимум
    if (freeHeap < requiredHeap)
    {
        context->logger->print("[OtaInst] Insufficient free heap: ");
        context->logger->print(freeHeap);
        context->logger->print(" bytes, need at least ");
        context->logger->println(requiredHeap);
        setError(NETWORK_ERROR);
        setState(ERROR);
        return false;
    }

    context->logger->print("[OtaInst] Starting upgrade from URL: ");
    context->logger->println(url);
    setState(DOWNLOADING);
    progress = 0;
    errorCode = NONE;

    if (stateSwitcher) {
        stateSwitcher->switchState(StateType::OtaUpdating);
    }

    if (updaterCallback) {
        updaterCallback->onUpgradeStart();
    }

#if defined(ESP8266) || defined(ESP32)
    HTTP_UPDATE.onProgress([this](int cur, int total) {
        if (total > 0)
        {
            this->progress = (cur * 100) / total;
            if (this->updaterCallback) 
            {
                this->updaterCallback->onUpgradeProgress(this->progress);
            }
            if (this->progress % 10 == 0)
            {
                this->context->logger->print("[OtaInst] Progress: ");
                this->context->logger->print(this->progress);
                this->context->logger->println("%");
            }
        }
    });

    HTTP_UPDATE.rebootOnUpdate(true);

    WiFiClient *selectedClient = context->clientProvider->getClient(url);

    context->logger->println("[OtaInst] Calling HTTP_UPDATE.update...");
    t_httpUpdate_return ret = HTTP_UPDATE.update(*selectedClient, url);
    context->logger->print("[OtaInst] HTTP_UPDATE.update returned: ");
    context->logger->println(ret);

    bool success = ret == HTTP_UPDATE_OK;
    if (success)
    {
        context->logger->print("[OtaInst] ");
        context->logger->println("Upgrade successful");
    }
    else
    {
        context->logger->print("[OtaInst] Upgrade failed, HTTP update return: ");
        context->logger->println(ret);
        setError(UPDATE_FAILED);
        setState(ERROR);
    }

    if (updaterCallback) 
    {
        updaterCallback->onUpgradeEnd(success);
    }
    return success;
#else
    setError(PLATFORM_UNSUPPORTED);
    setState(ERROR);
    context->logger->print("[OtaInst] ");
    context->logger->println("Platform unsupported");
    return false;
#endif
}

void OtaInstaller::reset()
{
    state = IDLE;
    errorCode = NONE;
    progress = 0;
    context->logger->println("[OtaInst] Reset");
}

void OtaInstaller::setState(State newState)
{
    state = newState;
}

String OtaInstaller::getError() const
{
    switch (errorCode)
    {
    case NONE:
        return "";
    case NETWORK_ERROR:
        return "Network error";
    case UPDATE_FAILED:
        return "Update failed";
    case PLATFORM_UNSUPPORTED:
        return "Platform unsupported";
    default:
        return "Unknown error";
    }
}

void OtaInstaller::setError(Error code)
{
    errorCode = code;
    context->logger->print("[OtaInst] Error code: ");
    context->logger->println(code);
}

void OtaInstaller::setStateSwitcher(StateSwitcher* switcher)
{
    stateSwitcher = switcher;
}