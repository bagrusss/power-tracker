#if defined(ESP8266)
#include <LittleFS.h>
#elif defined(ESP32)
#include <FS.h>
#include <LittleFS.h>
#define LITTLEFS LittleFS
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // default GPIO for ESP32 built-in LED
#endif

#include "Context.h"
#include "states/mc/McStateMachine.h"
#include "db/data.h"
#include "api/PowerMonitor.h"

#include "states/mc/ConnectingWifiState.h"
#include "states/mc/IdleState.h"
#include "states/mc/SetupWifiState.h"
#include "states/mc/SettingsState.h"
#include "states/mc/WarningState.h"

#include "devices/EspBoard.h"
#include "devices/EspWifiAdapter.h"
#include "devices/circuit/Ina219Impl.h"
#include "devices/circuit/Ina226Impl.h"
#include "devices/circuit/Ina231Impl.h"
#include "devices/circuit/InaDetector.h"
#include "devices/TemperatureMeterImpl.h"
#include "devices/DisplaySSD1306.h"
#include "devices/BoardDetector.h"

#include "net/HttpRequestsHandler.h"
#include "net/CommandsHandler.h"
#include "hack/WebUi.h"
#include "res/Titles.h"
#include "ota/OtaUpdater.h"

Context *context;
McStateMachine *stateMachine;
CommandsHandler *commandsHandler;
HttpRequestsHandler *requestsHandler;

void buildUi(sets::Builder &b)
{
  stateMachine->buildUI(b);
}

void updateUi(sets::Updater &upd)
{
  stateMachine->updateUi(upd);
}

void handleStatus(gson::Str &printer)
{
  auto state = stateMachine->getCurrentState();
  if (state)
  {
    state->printStatus(printer);
  }
}

void onWifiDisconnected()
{
  stateMachine->switchState(StateType::ConnectingWifi);
}

inline void initDb(GyverDBFile *const db)
{
#ifdef ESP32
  LittleFS.begin(true);
#else
  LittleFS.begin();
#endif

  db->begin();

  db->init(NET_CONFIG::wifi_ssid, "");
  db->init(NET_CONFIG::wifi_pass, "");

  db->init(DEVICE_INFO::name, UiTitles::DefaultValues::DEFAULT_NAME);
  db->init(DEVICE_INFO::os_info, UiTitles::DefaultValues::DEFAULT_NAME);
  db->init(DEVICE_INFO::serial_number, UiTitles::DefaultValues::DEFAULT_NAME);
  db->init(DEVICE_INFO::full_capacity, UiTitles::DefaultValues::DEFAULT_NAN);

  db->init(POWER_STRATEGY_CONFIG::battery_min_voltage, 3.3);
  db->init(POWER_STRATEGY_CONFIG::battery_max_voltage, 4.35);
  db->init(POWER_STRATEGY_CONFIG::power_source_min_voltage, 3.5);
  db->init(POWER_STRATEGY_CONFIG::power_source_max_voltage, 4);

  db->init(BOARD_CONFIG::i2c_frequency, 100);

  db->init(OTA_CONFIG::server_url, "");
  db->init(OTA_CONFIG::url_params, "");
  db->init(OTA_CONFIG::auto_update_enabled, false);
  db->init(OTA_CONFIG::check_interval, 0);
}

Context *const initContext(
    WifiAdapter *const wifiAdapter,
    std::map<uint8_t, std::unique_ptr<PowerMonitor>> *const monitors,
    WebUi *const webUi,
    GyverDBFile *const db,
    Print *const logger,
    EspBoard *board)
{
  auto *clientProvider = new WifiClientProvider();

  Context *const context = new Context(
      board,
      wifiAdapter,
      monitors,
      nullptr, // new TemperatureMeterImpl(0), // currently - unused
      webUi,
      db,
      new LedIndicator(LED_BUILTIN),
      new DisplaySSD1306(0x78 >> 1),
      clientProvider,
      logger
  );

  context->init();
  return context;
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  auto db = new GyverDBFile(&LittleFS, UiTitles::FileNames::DATA_DB);
  initDb(db);

  auto logger = &Serial;
  auto boardDetector = BoardDetector(logger);
  auto detectedConfig = boardDetector.detectBoard();

  auto board = new EspBoard(detectedConfig, logger);
  uint32_t freq = db->get(BOARD_CONFIG::i2c_frequency).toInt() * 1000;
  board->setI2CFreq(freq);

  auto detector = InaDetector(logger, detectedConfig);

  auto monitors = detector.detectDevices(0x40, 0x40 + 15, db);

  auto wifiAdapter = new EspWifiAdapter(logger);
  auto webUi = new WebUi(UiTitles::WebUi::TITLE, db);

  context = initContext(wifiAdapter, monitors, webUi, db, logger, board);

  stateMachine = new McStateMachine(context);
  commandsHandler = new CommandsHandler(context->inaMachines);

  context->otaInstaller->setStateSwitcher(stateMachine);

  requestsHandler = new HttpRequestsHandler(
      webUi->getServer(),
      commandsHandler);

  wifiAdapter->setDisconnectedCallback(&onWifiDisconnected);

  webUi->onBuild(buildUi);
  webUi->onUpdate(updateUi);

  requestsHandler->setStatusCallback(handleStatus);
  requestsHandler->begin();

  stateMachine->switchState(StateType::ConnectingWifi);
  context->logger->println(UiTitles::Messages::SETTED_UP);
}

void loop()
{
  stateMachine->handle();
  context->ledIndicator->update();
  context->settings->tick();

  context->otaUpdater->tick();
}