#include "State.h"
#include "db/data.h"
#include "states/ina/InaStateMachine.h"
#include "states/ina/InaStateMachines.h"

uint8_t State::activeTab = 0;

State::State(StateSwitcher *const stateMachine, Context *const context)
    : stateMachine(stateMachine),
      context(context)
{
    context->inaMachines->addListener(this);
}

State::~State()
{
    context->inaMachines->removeListener(this);
}

void State::onEnter()
{
    checkAndprintSensorAddresses();
}

uint8_t State::checkAndprintSensorAddresses()
{
    uint8_t active = 0;
    context->display->withUpdate([this, &active](PixelDisplay& d) {
        auto line = Lines::L5;
        auto size = context->monitors->size();
        if (size < 6) {
            line = Lines::L7;
        } else if (size < 11) {
            line = Lines::L6;
        }
        d.selectLine(line);
        bool first = true;
        for (const auto &pair : *(context->monitors))
        {
            if (!first)
            {
                d.print(";");
            }
            uint8_t address = pair.first;
            InaStateMachine *const machine = context->inaMachines->getInaStateMachine(address);
            bool isBusy = machine && machine->isBusy();
            if (isBusy)
            {
                ++active;
            }
            String addrStr = "x" + String(address, HEX);
            d.print(addrStr, isBusy);
            first = false;
        }
    });
    return active;
}

void State::printStatus(gson::Str &printer) const
{
    printer["status"] = getDescription();
}

String State::getSensorTabLabels() const
{
    String tabLabels = "";
    size_t estimatedSize = context->monitors->size() * 5; // "0xXX;" в среднем 5 символов на датчик
    tabLabels.reserve(estimatedSize);

    size_t i = 0;
    for (const auto &pair : *(context->monitors))
    {
        if (i > 0)
        {
            tabLabels += ";";
        }
        tabLabels += "0x";
        tabLabels += String(pair.first, HEX);
        i++;
    }
    return tabLabels;
}

uint8_t State::getSensorAddressForTab(const uint8_t &tabIndex) const
{
    uint8_t i = 0;
    for (const auto &pair : *(context->monitors))
    {
        if (i == tabIndex)
        {
            return pair.first;
        }
        i++;
    }
    return 0;
}

bool State::isTabAvailable(const uint8_t &tabIndex) const
{
    return tabIndex < context->monitors->size();
}

void State::updateActiveDevice()
{
    uint8_t activeAddress = getSensorAddressForTab(activeTab);
    auto it = context->monitors->find(activeAddress);
    if (it != context->monitors->end() && it->second)
    {
        activeDevice = it->second.get();
    }
    else
    {
        activeDevice = nullptr;
    }
}

void State::onSensorStatusChanged(const uint8_t address, const bool isBusy)
{
    checkAndprintSensorAddresses();
}
