#include "InaTrackingState.h"

#include <GSON.h>
#include "db/data.h"
#include "util/Util.h"
#include "res/Titles.h"

#define HOUR_MS 3600000.0f

#define TRACKING_TIME_MS (this->endTime - this->startTime) / 1000

InaTrackingState::InaTrackingState(InaStateMachine *const stateMachine, Context *const context, PowerMonitor *const sensor)
    : InaState(stateMachine, context, sensor)
{
}

void InaTrackingState::buildUi(sets::Builder &b)
{
    InaState::buildUi(b);

    auto time = TRACKING_TIME_MS;
    b.Time(TRACKING_VALUES::tracking_time, UiTitles::TRACKING_TIME, &time);

    b.Label(BARRETY_INFO::voltage, UiTitles::VOLTAGE, withUnit(this->currentVoltage, UNIT_V));
    b.Label(BARRETY_INFO::current, UiTitles::CURRENT, withUnit(this->currentCurrent, UNIT_mA, UNIT_A));

    b.Label(TRACKING_VALUES::totalCurrent, UiTitles::TOTAL_CURRENT, withUnit(this->totalCurrent, UNIT_mAh, UNIT_Ah), sets::Colors::Red);
    b.Label(TRACKING_VALUES::totalPower, UiTitles::TOTAL_ENERGY, withUnit(this->totalPower, UNIT_mWh, UNIT_Wh), sets::Colors::Orange);
    b.Label(TRACKING_VALUES::maxPower, UiTitles::MAX_POWER, withUnit(this->maxPower, UNIT_mW, UNIT_W));
    b.Label(TRACKING_VALUES::maxCurrent, UiTitles::MAX_CURRENT, withUnit(this->maxCurrent, UNIT_mA, UNIT_A));
    b.Label(TRACKING_VALUES::minVoltage, UiTitles::MIN_VOLTAGE, withUnit(this->minVoltage, UNIT_V));

    if (b.beginRow())
    {
        if (!isTracking && b.Button(UiTitles::UiButtons::BACK, sets::Colors::Gray))
        {
            stateMachine->switchState(static_cast<size_t>(InaStateType::Idle));
            b.reload();
        }

        if (b.Button(isTracking ? UiTitles::UiButtons::STOP : UiTitles::UiButtons::RESTART, isTracking ? sets::Colors::Red : sets::Colors::Green))
        {
            isTracking = !isTracking;
            if (!isTracking)
            {
                stop();
            }
            else
            {
                reset();
            }
            b.reload();
        }

        b.endRow();
    }
}

void InaTrackingState::updateUi(sets::Updater &upd)
{
    InaState::updateUi(upd);

    upd.update(TRACKING_VALUES::tracking_time, TRACKING_TIME_MS);

    upd.update(BARRETY_INFO::voltage, withUnit(this->currentVoltage, UNIT_V));
    upd.update(BARRETY_INFO::current, withUnit(this->currentCurrent, UNIT_mA, UNIT_A));

    upd.update(TRACKING_VALUES::totalCurrent, withUnit(this->totalCurrent, UNIT_mAh, UNIT_Ah));
    upd.update(TRACKING_VALUES::totalPower, withUnit(this->totalPower, UNIT_mWh, UNIT_Wh));

    upd.update(TRACKING_VALUES::maxCurrent, withUnit(this->maxCurrent, UNIT_mA, UNIT_A));
    upd.update(TRACKING_VALUES::maxPower, withUnit(this->maxPower, UNIT_mW, UNIT_W));
    upd.update(TRACKING_VALUES::minVoltage, withUnit(this->minVoltage, UNIT_V));

    if (informAboutResults)
    {
        informAboutResults = false;
        String filename = UiTitles::FileNames::RESULTS_SENSOR_PREFIX;
        filename += String(stateMachine->getSensorAddress(), HEX);
        filename += ".json";
        upd.notice(String(UiTitles::Sensor::RESULTS_SAVED_TO) + filename);
    }
}

void InaTrackingState::onEnter()
{
    reset();
    isTracking = true;
}

void InaTrackingState::handle()
{
    auto currentTime = millis();
    auto interval = currentTime - lastTime;
    if (interval > measurementInterval)
    {
        lastTime = currentTime;

        currentVoltage = sensor->getLoadVoltage();
        currentCurrent = abs(sensor->getCurrent());
        currentPower = sensor->getPower();

        if (!isTracking)
            return;

        endTime = currentTime;

        auto elapsedTime = interval / HOUR_MS;
        totalCurrent += currentCurrent * elapsedTime;
        totalPower += currentPower * elapsedTime;

        if (currentVoltage < minVoltage)
        {
            minVoltage = currentVoltage;
        }

        if (currentCurrent > maxCurrent)
        {
            maxCurrent = currentCurrent;
        }

        if (currentPower > maxPower)
        {
            maxPower = currentPower;
        }
    }
}

void InaTrackingState::onLeft()
{
    stop();
}

void InaTrackingState::printStatus(gson::Str &printer) const
{
    InaState::printStatus(printer);

    auto totalTime = endTime - startTime;
    printer["totalTime"] = totalTime / 1000;
    printer["sensorAddr"] = stateMachine->getSensorAddress();

    printer["currentCurrent"] = currentCurrent;
    printer["totalCurrent"] = totalCurrent;
    printer["maxCurrent"] = maxCurrent;

    printer["currentVoltage"] = currentVoltage;
    printer["minVoltage"] = minVoltage;

    printer["currentPower"] = currentPower;
    printer["totalPower"] = totalPower;
    printer["maxPower"] = maxPower;

    printer["interval"] = measurementInterval;
}

InaStateType InaTrackingState::getStateType() const
{
    return InaStateType::Tracking;
}

void InaTrackingState::reset()
{
    startTime = millis();
    endTime = startTime;
    lastTime = startTime;

    minVoltage = sensor->getLoadVoltage(false);

    maxCurrent = 0;
    maxPower = 0;
    totalPower = 0;
    totalCurrent = 0;

    currentVoltage = minVoltage;
    currentCurrent = 0;
    currentPower = 0;

    measurementInterval = context->db->get(BAT_MONITOR_CONFIG::measurement_interval);
}

void InaTrackingState::stop()
{
    endTime = millis();
    informAboutResults = true;

    String filename = UiTitles::FileNames::RESULTS_SENSOR_PREFIX;
    filename += String(stateMachine->getSensorAddress(), HEX);
    filename += ".json";
    saveResults(const_cast<char *>(filename.c_str()));
}

void InaTrackingState::saveResults(const char *const path)
{
    gson::Str res;
    printStatus(res);

    if (LittleFS.exists(path))
    {
        LittleFS.remove(path);
    }

    File file = LittleFS.open(path, "w");
    if (file)
    {
        file.print(res);
        file.close();
    }
    else
    {
        context->logger->println("Failed to open file for writing");
    }
}

const char *InaTrackingState::getDescription() const
{
    static char description[32];
    snprintf(description, sizeof(description), "Tracking 0x%02X", stateMachine->getSensorAddress());
    return description;
}
