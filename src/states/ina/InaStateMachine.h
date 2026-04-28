#pragma once

#include "InaState.h"
#include "../mc/State.h"

class Context;

class InaStateMachine : public StateSwitcher
{
public:
    InaStateMachine(Context *const context, uint8_t sensorAddress);
    virtual ~InaStateMachine();

    virtual void switchState(const size_t stateId) override;

    void handle() const;
    void buildUI(sets::Builder &b) const;
    void updateUi(sets::Updater &upd) const;

    InaState *const getCurrentState() const;
    bool isBusy() const;

    uint8_t getSensorAddress() const;

private:
    Context *const context;
    uint8_t sensorAddress;
    InaState *currentState = nullptr;

    std::vector<InaState *> states;

    InaState *const createState(InaStateType stateType);
};
