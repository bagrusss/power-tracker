#pragma once

#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

class WifiClientProvider
{
public:
    WifiClientProvider(){};
    ~WifiClientProvider();

    WiFiClient *const getClient(const String &url);
    WiFiClient *const getClient(const char *url);

    void cleanup();

private:
    WiFiClient *httpClient = nullptr;
    WiFiClientSecure *httpsClient = nullptr;

    void ensureHttpClient();
    void ensureHttpsClient();
};