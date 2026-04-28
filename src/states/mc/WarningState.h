#pragma once

#include "State.h"

class WarningState : public State
{
public:
    WarningState(StateSwitcher *const stateMachine, Context *const context);

    void buildUi(sets::Builder &b) override;
    void updateUi(sets::Updater &upd) override;

    void onEnter() override;
    void handle() override;

    const char *getDescription() const override;
};
