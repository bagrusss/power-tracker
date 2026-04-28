#pragma once

#include "CommandsHandler.h"

#include <SettingsAsync.h>
#include <GSON.h>

class HttpRequestsHandler
{
public:
    HttpRequestsHandler(
        AsyncWebServer *const s,
        CommandsHandler *const h);

    typedef std::function<void(gson::Str &p)> StatusCallback;

    void begin();
    void setStatusCallback(StatusCallback cb);

private:
    AsyncWebServer *const server;
    CommandsHandler *const handler;

    StatusCallback statusCb = nullptr;
    gson::Str printer;

    void handleStatus(AsyncWebServerRequest *const request);
    void handleStart(AsyncWebServerRequest *const request);
    void handleStop(AsyncWebServerRequest *const request);
    void handleAll(AsyncWebServerRequest *const request);
    void print404(AsyncWebServerRequest *const request);
};