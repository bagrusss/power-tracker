#pragma once

#include "State.h"


/*
 * В этом состоянии настраивается настраивается сеть и возможно 2 ситуации:
 * - успешное подключение к точке доступа,
 * - поднятие собственной точки доступа
 * В обоих случаях нужно вывести на дисплей креды для подключения к МК и замеров 
 */
class ConnectingWifiState : public State
{
public:
    ConnectingWifiState(StateSwitcher *const stateMachine, Context *const context);

    void buildUi(sets::Builder &b) override;

    void onEnter() override;
    void onLeft() override;
    void handle() override;

    const char *getDescription() const override;

private:
    bool connectToWiFi(const char *ssid, const char *pass) const;
    void showWifiInfo() const;
    void startWifiAp();

    void prepareWifiName(char *const source, const size_t size) const;
    void printSerialInfo(const String &apHelp, const String &ipHelp) const;
    void printDisplayInfo(const String &apHelp, const String &ipHelp) const;
};
