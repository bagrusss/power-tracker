#pragma once

#include <GyverDBFile.h>
#include <SettingsAsync.h>
#include <map>

#include "api/Board.h"
#include "api/WifiAdapter.h"
#include "api/PowerMonitor.h"
#include "api/TemperatureMeter.h"
#include "api/LedIndicator.h"
#include "api/PixelDisplay.h"
#include "devices/circuit/InaDetector.h"
#include "ota/OtaInstaller.h"
#include "ota/OtaUpdater.h"
#include "net/WifiClientProvider.h"

class InaStateMachines;
class OtaUpdater;

class Context
{
public:
    Board *const board;
    WifiAdapter *const wifi;
    std::map<uint8_t, std::unique_ptr<PowerMonitor>> *const monitors;
    TemperatureMeter *const temperatureMeter;
    SettingsAsync *const settings;
    GyverDBFile *const db;
    LedIndicator *const ledIndicator;
    PixelDisplay *const display;
    OtaInstaller *const otaInstaller;
    OtaUpdater *const otaUpdater;
    InaStateMachines *const inaMachines;
    WifiClientProvider *const clientProvider;
    Print *const logger;

    Context(
        Board *const b,
        WifiAdapter *const wf,
        std::map<uint8_t, std::unique_ptr<PowerMonitor>> *const deviceMonitors,
        TemperatureMeter *const t,
        SettingsAsync *const s,
        GyverDBFile *const database,
        LedIndicator *const ld,
        PixelDisplay *const disp,
        WifiClientProvider *const clientProvider,
        Print *const log = nullptr);
    ~Context();

    void init();
};
