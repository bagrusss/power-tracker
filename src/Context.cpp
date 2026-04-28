#include "Context.h"

#include "db/data.h"
#include "res/Titles.h"
#include "util/Prefs.h"
#include "states/ina/InaStateMachines.h"
#include "ota/OtaUpdater.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // default GPIO for ESP32 built-in LED
#endif

Context::Context(
    Board *const b,
    WifiAdapter *const wf,
    std::map<uint8_t, std::unique_ptr<PowerMonitor>> *const deviceMonitors,
    TemperatureMeter *const t,
    SettingsAsync *const s,
    GyverDBFile *const database,
    LedIndicator *const ld,
    PixelDisplay *const disp,
    WifiClientProvider *const clientProvider,
    Print *const log)
    : board(b),
      wifi(wf),
      monitors(deviceMonitors),
      temperatureMeter(t),
      settings(s),
      db(database),
      ledIndicator(ld),
      display(disp),
      otaInstaller(new OtaInstaller(this)),
      otaUpdater(new OtaUpdater(this)),
      inaMachines(new InaStateMachines(this)),
      clientProvider(clientProvider),
      logger(log)
{
}

Context::~Context()
{
    delete inaMachines;

    monitors->clear();
    delete monitors;

    delete board;
    delete wifi;
    delete temperatureMeter;
    delete settings;
    delete db;
    delete ledIndicator;
    delete display;
    delete otaUpdater;
    delete otaInstaller;
    delete clientProvider;
}

void Context::init()
{
    pinMode(LED_BUILTIN, OUTPUT);

    const auto config = board->getConfig();
    display->init(config->sdaPin, config->sclPin);
    display->setContrast(0x80);

    settings->begin();
    settings->setProjectInfo(UiTitles::WebUi::TITLE);
    settings->setUpdatePeriod(1000);

    inaMachines->init();

    otaUpdater->begin();

    if (monitors->empty())
    {
        display->selectLine(Lines::L7);
        display->print(UiTitles::Messages::INA_NOT_FOUND);
        display->update();
    }

    logger->println("context ready");
}
