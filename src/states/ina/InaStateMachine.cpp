#include "InaStateMachine.h"

#include "InaIdleState.h"
#include "InaTrackingState.h"
#include "InaStateMachines.h"

InaStateMachine::InaStateMachine(Context *const context, uint8_t sensorAddress)
    : context(context), sensorAddress(sensorAddress), currentState(nullptr)
{
    states.resize((size_t)InaStateType::SIZE, nullptr);
    switchState((size_t)InaStateType::Idle);
}

InaStateMachine::~InaStateMachine()
{
    for (auto s : states)
    {
        delete s;
    }
    states.clear();
}

void InaStateMachine::switchState(const size_t stateId)
{
    InaState *newState = nullptr;

    if (stateId < (size_t)InaStateType::SIZE)
    {
        newState = createState((InaStateType)stateId);
    }

    if (newState && currentState != newState)
    {
        if (currentState)
        {
            currentState->onLeft();
        }
        currentState = newState;
        currentState->onEnter();

        context->logger->printf("INA 0x%02X new state: %d | %s, free heap: %d\n", sensorAddress, stateId, currentState->getDescription(), context->board->getFreeHeap());
        context->inaMachines->notifySensorStatusChanged(sensorAddress, isBusy());
    }
}

void InaStateMachine::handle() const
{
    if (currentState)
    {
        currentState->handle();
    }
}

void InaStateMachine::buildUI(sets::Builder &b) const
{
    if (currentState)
    {
        currentState->buildUi(b);
    }
}

void InaStateMachine::updateUi(sets::Updater &upd) const
{
    if (currentState)
    {
        currentState->updateUi(upd);
    }
}

InaState *const InaStateMachine::getCurrentState() const
{
    return currentState;
}

bool InaStateMachine::isBusy() const
{
    return currentState != nullptr && currentState == states[(size_t)InaStateType::Tracking];
}

uint8_t InaStateMachine::getSensorAddress() const
{
    return sensorAddress;
}

InaState *const InaStateMachine::createState(InaStateType stateType)
{
    InaState *newState = nullptr;

    PowerMonitor* sensor = nullptr;
    auto it = context->monitors->find(sensorAddress);
    if (it != context->monitors->end() && it->second && it->second->isPrepared()) {
        sensor = it->second.get();
    }

    switch (stateType)
    {
    case InaStateType::Idle:
        if (!states[(size_t)InaStateType::Idle])
        {
            states[(size_t)InaStateType::Idle] = new InaIdleState(this, context, sensor);
        }
        newState = states[(size_t)InaStateType::Idle];
        break;
    case InaStateType::Tracking:
        if (!states[(size_t)InaStateType::Tracking])
        {
            states[(size_t)InaStateType::Tracking] = new InaTrackingState(this, context, sensor);
        }
        newState = states[(size_t)InaStateType::Tracking];
        break;
    default:
        context->logger->printf("InaStateMachine cannot create state %d\n", (uint8_t)stateType);
        return nullptr;
    }

    return newState;
}