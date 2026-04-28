#pragma once

#include "State.h"

#include "devices/circuit/InaDetector.h"
#include "states/ina/InaStateMachine.h"
#include <vector>

class IdleState : public State
{
public:
    IdleState(StateSwitcher *const stateMachine, Context *const context);
    void buildUi(sets::Builder &b) override;
    void updateUi(sets::Updater &upd) override;

    void onEnter() override;
    void onLeft() override;
    void handle() override;

    void onSensorStatusChanged(const uint8_t address, const bool isBusy) override;

    const char *getDescription() const override;

private:
    bool ledState = false;
    bool confirmStart = false;
    InaStateMachine *activeInaMachine = nullptr;
    bool ledBlinkFast = false;

    void updateActiveInaMachine();
    void updateBattery(sets::Updater &upd, Battery *const bt);
    void updatePowerSource(sets::Updater &upd, PowerSource *const ps);
    void updateLedBlink(const uint8_t &activeSensors);
};
