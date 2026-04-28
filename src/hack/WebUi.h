#pragma once

#include <SettingsAsync.h>

class WebUi : public SettingsAsync
{
public:
#ifndef SETT_NO_DB
    WebUi(const String &title = "", GyverDB *db = nullptr) : SettingsAsync(title, db) {}
#else
    WebUi(const String &title = "") : SettingsAsync(title) {}
#endif

    AsyncWebServer *const getServer()
    {
        return &server;
    }
};
