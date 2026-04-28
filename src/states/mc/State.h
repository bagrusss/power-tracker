#pragma once

#include <GSON.h>

#include "Context.h"
#include "devices/circuit/InaDetector.h"
#include "api/SensorStatusListener.h"

enum StateType
{
    ConnectingWifi,
    Idle,
    SetupWifi,
    Settings,
    Warning,
    OtaUpdating,

    SIZE,
};

class StateSwitcher
{
public:
    virtual ~StateSwitcher() = default;
    virtual void switchState(const size_t stateId) = 0;
};

class State : public SensorStatusListener
{

public:
    State(StateSwitcher *const stateSwitcher, Context *const context);
    virtual ~State();

    virtual void buildUi(sets::Builder &b) {};
    virtual void updateUi(sets::Updater &upd) {};

    virtual void onEnter();
    virtual void onLeft() {}
    virtual void handle() {}

    virtual void printStatus(gson::Str &printer) const;

    virtual const char *getDescription() const
    {
        return "";
    }

    void onSensorStatusChanged(const uint8_t address, const bool isBusy) override;

protected:
    StateSwitcher *const stateMachine;
    Context *const context;
    PowerMonitor *activeDevice = nullptr;
    static uint8_t activeTab;

    String getSensorTabLabels() const;
    uint8_t getSensorAddressForTab(const uint8_t &tabIndex) const;
    bool isTabAvailable(const uint8_t &tabIndex) const;

    void updateActiveDevice();
    uint8_t checkAndprintSensorAddresses();
};
