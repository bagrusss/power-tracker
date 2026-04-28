#include "ConnectingWifiState.h"

#include "util/icons.h"
#include "res/Titles.h"

#include "db/data.h"

#define MAX_WIFI_ATTEMPTS 100


ConnectingWifiState::ConnectingWifiState(StateSwitcher *const stateMachine, Context *const context)
    : State(stateMachine, context) {}

void ConnectingWifiState::buildUi(sets::Builder &b)
{
    b.Label(UiTitles::Messages::CONNECTING_TO_WIFI);
}

void ConnectingWifiState::onEnter()
{
    State::onEnter();
    context->ledIndicator->setEnabled(true);
}

void ConnectingWifiState::onLeft()
{
    context->ledIndicator->setEnabled(false);
}

void ConnectingWifiState::handle()
{
    const String ssid = context->db->get(NET_CONFIG::wifi_ssid);
    const String pass = context->db->get(NET_CONFIG::wifi_pass);
    if (ssid.length() > 0 && pass.length() > 0)
    {
        if (connectToWiFi(ssid.c_str(), pass.c_str()))
        {
            showWifiInfo();
            stateMachine->switchState(StateType::Idle);
        }
        else
        {
            startWifiAp();
            stateMachine->switchState(StateType::Idle);
        }
    }
    else
    {
        startWifiAp();
        stateMachine->switchState(StateType::SetupWifi);
    }
}


bool ConnectingWifiState::connectToWiFi(const char *ssid, const char *pass) const
{
    context->wifi->connectToAp(ssid, pass);
    uint8_t attempts = MAX_WIFI_ATTEMPTS;
    context->logger->print(UiTitles::Messages::CONNECTING_TO);
    context->logger->println(ssid);

    auto enaled = true;

    auto icon = tracker_icons::WIFI;
    context->display->withUpdate([this, &icon, ssid](PixelDisplay& d) {
        d.selectLine(Lines::L0);
        d.drawIcon(&icon);
        d.print(ssid);
    });
    
    do
    {
        context->logger->print('.');

        enaled = !enaled;
        context->ledIndicator->setConstantOn(enaled);

        context->display->withUpdate([this, &icon, enaled](PixelDisplay& d) {
            d.selectLine(Lines::L0);
            d.drawIcon(&icon, !enaled);
        });

        if (context->wifi->isWifiConnected())
        {
            context->logger->println();
            context->logger->print(UiTitles::Messages::CONNECTED_TO_WIFI_IP);
            context->logger->println(context->wifi->localIp());

            delay(500);

            return true;
        }

        delay(250);

        --attempts;
    } while (attempts > 0);

    context->logger->println();
    context->logger->print(UiTitles::Messages::FAILED_TO_CONNECT_TO);
    context->logger->println(ssid);
    return false;
}

void ConnectingWifiState::showWifiInfo() const
{
    auto wifi = tracker_icons::WIFI;
    context->display->withUpdate([this, &wifi](PixelDisplay& d) {
        d.selectLine(Lines::L0);
        d.drawIcon(&wifi);
        d.print(context->wifi->getSsid());
        d.selectLine(Lines::L1);
        d.print(context->wifi->localIp());
    });

    context->ledIndicator->setBlinkDurations(500, 2000);
    context->ledIndicator->setEnabled(true);
}

void ConnectingWifiState::startWifiAp()
{
    char name[12];
    prepareWifiName(name, sizeof(name));
    if (context->wifi->startAp(name, UiTitles::Network::DEFAULT_WIFI_PASS))
    {
        auto apHelp = String(UiTitles::Network::AP_PREFIX) + name;
        auto ipHelp = String(UiTitles::Network::IP_PREFIX) + context->wifi->apIp();

        printSerialInfo(apHelp, ipHelp);
        printDisplayInfo(apHelp, ipHelp);

        context->ledIndicator->setConstantOn(true);
    }
    else
    {
        auto text = UiTitles::Messages::COULDNT_UP_AP;
        context->logger->println(text);
        context->display->withUpdate([this, text](PixelDisplay& d) {
            d.selectLine(Lines::L0);
            d.print(text);
        });
    }
}

const char *ConnectingWifiState::getDescription() const
{
    return UiTitles::Messages::CONNECTING_WIFI;
}

void ConnectingWifiState::prepareWifiName(char *const source, const size_t size) const
{
    uint8_t mac[6];
    context->wifi->getMacAddress(mac);
    snprintf(source, size, "%s%02X%02X%02X", UiTitles::Network::TMP_WIFI_NAME, mac[3], mac[4], mac[5]);
}

void ConnectingWifiState::printSerialInfo(const String &apHelp, const String &ipHelp) const
{
    context->logger->print(apHelp);
    context->logger->print(" | ");
    context->logger->println(ipHelp);
}

void ConnectingWifiState::printDisplayInfo(const String &apHelp, const String &ipHelp) const
{
    context->display->withUpdate([this, apHelp, ipHelp](PixelDisplay& d) {
        d.selectLine(Lines::L0);
        auto wifi = tracker_icons::WIFI;
        d.drawIcon(&wifi);
        d.print(apHelp);
        d.selectLine(Lines::L1);
        d.print(ipHelp);
    });
}
