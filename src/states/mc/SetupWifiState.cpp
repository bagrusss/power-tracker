#include "SetupWifiState.h"

#include "db/data.h"
#include "res/Titles.h"

#define STATUS_SETUP_WIFI_TEXT = "Set up Wi-Fi AP!"

SetupWifiState::SetupWifiState(StateSwitcher *const stateMachine, Context *const context)
    : State(stateMachine, context) {}

void SetupWifiState::buildUi(sets::Builder &b)
{
    if (b.beginGroup(UiTitles::Groups::WIFI_CONFIG))
    {
        b.Input(NET_CONFIG::wifi_ssid, UiTitles::SSID);
        b.Pass(NET_CONFIG::wifi_pass, UiTitles::PASSWORD);
        if (b.Button(BUTTON_APPLY, UiTitles::UiButtons::APPLY_AND_REBOOT))
        {
            context->db->update();
            context->board->reboot();
        }

        if (b.Button(BUTTON_SKIP, UiTitles::UiButtons::SKIP))
        {
            stateMachine->switchState(StateType::Idle);
            b.reload();
        }
        b.endGroup();
    }
}

const char *SetupWifiState::getDescription() const
{
    return "SetupWifi";
}