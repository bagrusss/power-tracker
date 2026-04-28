#pragma once

#include "State.h"

class SetupWifiState : public State
{
public:
    SetupWifiState(StateSwitcher *const stateMachine, Context *const context);

    void buildUi(sets::Builder &b) override;

    const char *getDescription() const override;
};
