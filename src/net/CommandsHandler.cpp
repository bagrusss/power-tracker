#include "CommandsHandler.h"

#include <GSON.h>

#include "res/Titles.h"

#define CMD_START "start"
#define CMD_STOP "stop"
#define CMD_STATUS "status"

CommandsHandler::CommandsHandler(InaStateMachines *const inaMachines) : inaMachines(inaMachines) {}

InaStateMachine *CommandsHandler::getMachine(const String &id) const
{
    return inaMachines->getInaStateMachine(id);
}

void CommandsHandler::fillSuccessResponse(const String &operation, const String &stateMachineId, gson::Str &doc)
{
    doc["result"] = 1;
    doc["operation"] = operation;
    doc["serial"] = stateMachineId;
}

void CommandsHandler::collectStatusInfo(const String &stateMachineId, InaStateMachine *machine, gson::Str &doc)
{
    auto currentState = machine->getCurrentState();
    if (currentState)
    {
        currentState->printStatus(doc);
    }
}

void CommandsHandler::handleStatus(const String &stateMachineId, StatusCallback cb, gson::Str &doc)
{
    auto machine = getMachine(stateMachineId);
    if (!machine)
    {
        return;
    }
    fillSuccessResponse(CMD_STATUS, stateMachineId, doc);
    collectStatusInfo(stateMachineId, machine, doc);
    if (cb)
    {
        cb(doc);
    }
}

void CommandsHandler::handleStart(const String &stateMachineId, gson::Str &doc)
{
    auto machine = getMachine(stateMachineId);
    if (machine && machine->getCurrentState()->getStateType() != InaStateType::Tracking)
    {
        machine->switchState((size_t)InaStateType::Tracking);
    }

    fillSuccessResponse(CMD_START, stateMachineId, doc);
}

void CommandsHandler::handleStop(const String &stateMachineId, gson::Str &doc)
{
    auto machine = getMachine(stateMachineId);
    if (machine && machine->getCurrentState()->getStateType() == InaStateType::Tracking)
    {
        collectStatusInfo(stateMachineId, machine, doc);
        machine->switchState((size_t)InaStateType::Idle);
    }
    fillSuccessResponse(CMD_STOP, stateMachineId, doc);
}

void CommandsHandler::handleAll(gson::Str &doc)
{
    inaMachines->getAllSensorsInfo(doc);
}