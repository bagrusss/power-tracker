#include "InaStateMachines.h"
#include "Context.h"
#include "db/data.h"
#include "res/Titles.h"
#include <GSON.h>

InaStateMachines::InaStateMachines(Context *const context)
    : context(context)
{
}

void InaStateMachines::init()
{
    for (const auto &pair : *(context->monitors))
    {
        uint8_t address = pair.first;

        InaStateMachine *newMachine = new InaStateMachine(context, address);
        inaStateMachines[address] = newMachine;

        String deviceId = context->db->get(withKey(DEVICE_INFO::serial_number, address));
        if (deviceId.length() > 0)
        {
            sensorIdToAddress[deviceId] = address;
        }
        else
        {
            String hexAddress = UiTitles::DefaultValues::SN_PREFIX + String(address, HEX);
            sensorIdToAddress[hexAddress] = address;
        }

        context->logger->print("Created InaStateMachine for sensor 0x");
        context->logger->print(address, HEX);
        context->logger->print(" with ID: ");
        context->logger->println(deviceId);
    }
}

InaStateMachines::~InaStateMachines()
{
    for (auto &pair : inaStateMachines)
    {
        delete pair.second;
    }
    inaStateMachines.clear();
}

InaStateMachine *InaStateMachines::getInaStateMachine(const uint8_t &sensorAddress)
{
    auto it = inaStateMachines.find(sensorAddress);
    if (it != inaStateMachines.end())
    {
        return it->second;
    }

    context->logger->print("InaStateMachine not found for sensor 0x");
    context->logger->println(sensorAddress, HEX);
    return nullptr;
}

InaStateMachine *InaStateMachines::getInaStateMachine(const String &sensorId)
{
    auto it = sensorIdToAddress.find(sensorId);
    if (it != sensorIdToAddress.end())
    {
        return getInaStateMachine(it->second);
    }

    for (const auto &pair : *(context->monitors))
    {
        uint8_t address = pair.first;
        String deviceIdFromDb = context->db->get(withKey(DEVICE_INFO::serial_number, address));

        if (deviceIdFromDb == sensorId)
        {
            sensorIdToAddress[sensorId] = address;
            return getInaStateMachine(address);
        }
    }

    context->logger->print("Cannot find InaStateMachine for sensor ID: ");
    context->logger->println(sensorId);
    return nullptr;
}

void InaStateMachines::addListener(SensorStatusListener* listener)
{
    listeners.push_back(listener);
}

void InaStateMachines::removeListener(SensorStatusListener* listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
    {
        listeners.erase(it);
    }
}

void InaStateMachines::notifySensorStatusChanged(uint8_t address, bool isBusy)
{
    for (auto listener : listeners)
    {
        listener->onSensorStatusChanged(address, isBusy);
    }
}

bool InaStateMachines::isAnySensorBusy() const
{
    for (const auto &pair : inaStateMachines)
    {
        InaStateMachine *machine = pair.second;
        if (machine != nullptr && machine->isBusy())
        {
            return true;
        }
    }
    return false;
}

String InaStateMachines::getSensorId(uint8_t address) const
{
    for (const auto &pair : sensorIdToAddress)
    {
        if (pair.second == address)
        {
            return pair.first;
        }
    }
    String deviceId = context->db->get(withKey(DEVICE_INFO::serial_number, address));
    if (deviceId.length() > 0)
    {
        return deviceId;
    }
    return UiTitles::DefaultValues::SN_PREFIX + String(address, HEX);
}

void InaStateMachines::getAllSensorsInfo(gson::Str &doc) const
{
    doc["sensors"]('[');
    for (const auto &pair : inaStateMachines)
    {
        uint8_t address = pair.first;
        InaStateMachine *machine = pair.second;
        if (!machine) continue;

        doc('{');
        doc["address"] = address;
        doc["serial"] = getSensorId(address);
        doc["state"] = machine->isBusy() ? "tracking" : "idle";
        doc('}');
    }
    doc(']');
}