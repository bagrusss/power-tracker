#include "InaState.h"

#include "InaStateMachine.h"
#include "api/PowerMonitor.h"

InaState::InaState(InaStateMachine *const stateMachine, Context *const context, PowerMonitor *const sensor)
    : stateMachine(stateMachine), context(context), sensor(sensor)
{
}

void InaState::buildUi(sets::Builder &b) {}

void InaState::updateUi(sets::Updater &upd) {}

void InaState::printStatus(gson::Str &printer) const
{
    printer["type"] = getDescription();
    printer["stateType"] = static_cast<uint8_t>(getStateType());
}
