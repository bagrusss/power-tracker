#pragma once

#include <GSON.h>

#include "states/ina/InaStateMachines.h"
#include "states/ina/InaState.h"

class CommandsHandler
{
public:
    using StatusCallback = std::function<void(gson::Str &p)>;

    CommandsHandler(InaStateMachines *const inaMachines);

    void handleStatus(const String &stateMachineId, StatusCallback cb, gson::Str &doc);
    void handleStart(const String &stateMachineId, gson::Str &doc);
    void handleStop(const String &stateMachineId, gson::Str &doc);
    void handleAll(gson::Str &doc);

private:
    InaStateMachines *const inaMachines;

    InaStateMachine *getMachine(const String &id) const;
    void fillSuccessResponse(const String &operation, const String &stateMachineId, gson::Str &doc);
    void collectStatusInfo(const String &stateMachineId, InaStateMachine *machine, gson::Str &doc);
};