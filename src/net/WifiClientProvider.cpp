#include "WifiClientProvider.h"

WifiClientProvider::~WifiClientProvider()
{
    cleanup();
}

void WifiClientProvider::ensureHttpClient()
{
    if (httpClient == nullptr)
    {
        httpClient = new WiFiClient();
    }
}

void WifiClientProvider::ensureHttpsClient()
{
    if (httpsClient == nullptr)
    {
        httpsClient = new WiFiClientSecure();
#ifdef ESP8266
        httpsClient->setInsecure();
#endif
    }
}

WiFiClient *const WifiClientProvider::getClient(const String &url)
{
    if (url.startsWith("https://"))
    {
        ensureHttpsClient();
        return httpsClient;
    }
    else
    {
        ensureHttpClient();
        return httpClient;
    }
}

WiFiClient *const WifiClientProvider::getClient(const char *url)
{
    return getClient(String(url));
}

void WifiClientProvider::cleanup()
{
    if (httpClient != nullptr)
    {
        delete httpClient;
        httpClient = nullptr;
    }
    if (httpsClient != nullptr)
    {
        delete httpsClient;
        httpsClient = nullptr;
    }
}