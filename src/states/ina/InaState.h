#pragma once

#include <SettingsAsync.h>
#include <GSON.h>

#include "Context.h"

// Forward declarations
class InaStateMachine;
class PowerMonitor;

enum class InaStateType
{
    Idle,
    Tracking,

    SIZE,
};

class InaState
{
public:
    InaState(InaStateMachine *const stateMachine, Context *const context, PowerMonitor *const sensor);
    virtual ~InaState() = default;

    virtual void buildUi(sets::Builder &b);
    virtual void updateUi(sets::Updater &upd);

    virtual void onEnter() = 0;
    virtual void onLeft() {}
    virtual void handle() {}

    virtual void printStatus(gson::Str &printer) const;

    virtual const char *getDescription() const
    {
        return "";
    }

    virtual InaStateType getStateType() const = 0;

protected:
    InaStateMachine *const stateMachine;
    Context *const context;
    PowerMonitor *const sensor;
};