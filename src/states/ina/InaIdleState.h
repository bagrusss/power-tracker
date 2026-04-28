#pragma once

#include "InaState.h"
#include "InaStateMachine.h"

// Forward declaration
class StateSwitcher;

class InaIdleState : public InaState
{
public:
    InaIdleState(InaStateMachine *const stateMachine, Context *const context, PowerMonitor *const sensor);
    void buildUi(sets::Builder &b) override;
    void updateUi(sets::Updater &upd) override;

    void onEnter() override;
    void onLeft() override;

    const char *getDescription() const override;

    InaStateType getStateType() const override;

private:
    bool ledState = false;

    void drawBattery(sets::Builder &b, Battery *const bt, const float &voltage, const float &current);
    void drawPowerSource(sets::Builder &b, PowerSource *const ps, const float &voltage, const float &current);
    void drawSensorData(sets::Builder &b, PowerMonitor *const sensor);

    void updateBattery(sets::Updater &upd, Battery *const bt, const float &voltage, const float &current);
    void updatePowerSource(sets::Updater &upd, PowerSource *const ps);
    void updateSensorData(sets::Updater &upd, PowerMonitor *const sensor);
};