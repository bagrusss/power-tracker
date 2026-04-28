#include "WarningState.h"

WarningState::WarningState(StateSwitcher *const stateMachine, Context *const context)
    : State(stateMachine, context) {}

void WarningState::buildUi(sets::Builder &b)
{
}

void WarningState::updateUi(sets::Updater &upd)
{
}

void WarningState::onEnter()
{
}

void WarningState::handle()
{
}

const char *WarningState::getDescription() const
{
    return "Warning";
}