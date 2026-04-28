#include "McStateMachine.h"

#include "states/mc/ConnectingWifiState.h"
#include "states/mc/IdleState.h"
#include "states/mc/SetupWifiState.h"
#include "states/mc/SettingsState.h"
#include "states/mc/WarningState.h"
#include "states/mc/OtaUpdatingState.h"

McStateMachine::McStateMachine(Context *const context)
    : context(context)
{
    states.reserve(StateType::SIZE);

    states = {
        new ConnectingWifiState(this, context), // 0 - StateType::ConnectingWifi
        new IdleState(this, context),           // 1 - StateType::Idle
        new SetupWifiState(this, context),      // 2 - StateType::SetupWifi
        new SettingsState(this, context),       // 3 - StateType::Settings
        new WarningState(this, context),        // 4 - StateType::Warning
        new OtaUpdatingState(this, context),    // 5 - StateType::OtaUpdating
    };
}

McStateMachine::~McStateMachine()
{
    for (auto s : states)
    {
        delete s;
    }
    states.clear();
}

void McStateMachine::switchState(const size_t stateId)
{
    if (stateId >= states.size()) return;

    State *const newState = states[stateId];

    if (newState && currentState != newState)
    {
        if (currentState)
        {
            currentState->onLeft();
        }

        currentState = newState;
        newState->onEnter();
    }
}

void McStateMachine::handle() const
{
    if (currentState)
    {
        currentState->handle();
    }
}

void McStateMachine::buildUI(sets::Builder &b) const
{
    if (currentState)
    {
        currentState->buildUi(b);
    }
}

void McStateMachine::updateUi(sets::Updater &upd) const
{
    if (currentState)
    {
        currentState->updateUi(upd);
    }
}

State *const McStateMachine::getCurrentState() const
{
    return currentState;
}