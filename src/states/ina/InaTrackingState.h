#pragma once

#include <GSON.h>
#include "InaState.h"

#include "InaStateMachine.h"

class InaTrackingState : public InaState
{
public:
    InaTrackingState(InaStateMachine *const stateMachine, Context *const context, PowerMonitor *const sensor);
    ~InaTrackingState() = default;

    void buildUi(sets::Builder &b) override;
    void updateUi(sets::Updater &upd) override;

    void onEnter() override;
    void handle() override;
    void onLeft() override;

    const char *getDescription() const override;
    void printStatus(gson::Str &printer) const override;

    InaStateType getStateType() const override;

private:
    bool isTracking = false;

    size_t startTime = 0;
    size_t endTime = 0;
    size_t lastTime = 0;

    float totalCurrent = 0;
    float totalPower = 0;
    float minVoltage = 0;
    float maxPower = 0;
    float maxCurrent = 0;

    float currentVoltage = 0;
    float currentCurrent = 0;
    float currentPower = 0;

    uint16_t measurementInterval = 0;

    bool informAboutResults = false;

    void reset();
    void stop();
    void saveResults(const char *const path);

};