#pragma once

#include "states/mc/State.h"
#include "Context.h"

class McStateMachine : public StateSwitcher
{
public:
    McStateMachine(Context *const context);
    ~McStateMachine();

    void switchState(const size_t stateId) override;
    void handle() const;
    void buildUI(sets::Builder &b) const;
    void updateUi(sets::Updater &upd) const;

    State *const getCurrentState() const;

private:
    Context *const context;
    State *currentState = nullptr;

    std::vector<State *> states;
};