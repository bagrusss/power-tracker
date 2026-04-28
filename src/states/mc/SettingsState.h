#pragma once

#include "State.h"

class SettingsState : public State
{
public:
    SettingsState(StateSwitcher *const stateMachine, Context *const context);

    void buildUi(sets::Builder &b) override;

    void onEnter() override;
    void onLeft() override;

    const char *getDescription() const override;

private:
    bool wifiChanged = 0;
    bool curcuitChanged = 0;
};
