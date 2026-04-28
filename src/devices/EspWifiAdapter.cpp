#include "EspWifiAdapter.h"

#if defined(ESP32)
#include <WiFi.h>
#endif

EspWifiAdapter::EspWifiAdapter(Print *const logger) : logger(logger)
{
#if defined(ESP8266)
    WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected &event) {
        this->logger->print("WiFi disconnected: ");
        this->logger->println(event.reason);

        if (dcb) dcb();
    });
#elif defined(ESP32)
    disconnectHandlerId = WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        this->logger->print("WiFi disconnected: ");
        this->logger->println(info.wifi_sta_disconnected.reason);

        if (dcb) dcb();
    });
#endif
}

EspWifiAdapter::~EspWifiAdapter()
{
#if defined(ESP32)
    WiFi.removeEvent(disconnectHandlerId);
#endif
}

bool EspWifiAdapter::isWifiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void EspWifiAdapter::connectToAp(const char *ssid, const char *passwd)
{
    WiFi.mode(WiFiMode::WIFI_STA);
    WiFi.begin(ssid, passwd);
}

bool EspWifiAdapter::startAp(const char *ssid, const char *passwd)
{
    WiFi.mode(WiFiMode::WIFI_AP);
    return WiFi.softAP(ssid, passwd);
}

void EspWifiAdapter::getMacAddress(uint8_t *mac)
{
    WiFi.macAddress(mac);
}

String EspWifiAdapter::getSsid() const
{
    return WiFi.SSID();
}

String EspWifiAdapter::localIp()
{
    return WiFi.localIP().toString();
}

String EspWifiAdapter::apIp()
{
    return WiFi.softAPIP().toString();
}

void EspWifiAdapter::setDisconnectedCallback(DisconnectedCallback cb)
{
    this->dcb = cb;
}