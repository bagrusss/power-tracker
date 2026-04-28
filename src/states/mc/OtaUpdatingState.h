#pragma once

#include "states/mc/State.h"
#include "ota/OtaInstaller.h"

class OtaUpdatingState : public State, public OtaUpdaterCallback
{
public:
    OtaUpdatingState(StateSwitcher *const stateSwitcher, Context *const context);
    virtual ~OtaUpdatingState() = default;

    void onEnter() override;
    void onLeft() override;
    void buildUi(sets::Builder &b) override;
    void updateUi(sets::Updater &upd) override;
    void printStatus(gson::Str &printer) const override;
    const char *getDescription() const override;

    void onUpgradeStart() override;
    void onUpgradeProgress(const int progress) override;
    void onUpgradeEnd(const bool success) override;

private:
    int lastProgress = -1;
    bool upgradeFinished = false;
    bool upgradeSuccess = false;
    unsigned long lastUpdateTime = 0;
    int lastFilledWidth = 0;

    uint8_t barWidth;
    uint8_t barHeight;
    uint8_t barY;
};