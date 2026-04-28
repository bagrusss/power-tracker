#pragma once

#include "states/ina/InaStateMachine.h"
#include "api/SensorStatusListener.h"
#include <map>
#include <vector>

class Context;

class InaStateMachines
{
public:
    InaStateMachines(Context *const context);
    ~InaStateMachines();

    void init();

    InaStateMachine *getInaStateMachine(const uint8_t &sensorAddress);
    InaStateMachine *getInaStateMachine(const String &sensorId);

    void addListener(SensorStatusListener* listener);
    void removeListener(SensorStatusListener* listener);
    void notifySensorStatusChanged(uint8_t address, bool isBusy);

    bool isAnySensorBusy() const;

    void getAllSensorsInfo(gson::Str &doc) const;

private:
    Context *const context;
    std::map<uint8_t, InaStateMachine *> inaStateMachines;
    std::map<String, uint8_t> sensorIdToAddress;
    std::vector<SensorStatusListener*> listeners;

    String getSensorId(uint8_t address) const;
};