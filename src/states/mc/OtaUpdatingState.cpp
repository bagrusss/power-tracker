#include "OtaUpdatingState.h"

#include "Context.h"
#include "api/PixelDisplay.h"
#include "res/Titles.h"
#include "util/icons.h"

#define UPT_PROGRESS 1678

OtaUpdatingState::OtaUpdatingState(StateSwitcher *const stateSwitcher, Context *const context)
    : State(stateSwitcher, context),
      barWidth(context->display->width),
      barHeight(context->display->height / Lines::Lines_SIZE),
      barY(Lines::L4 * barHeight)
{
}

void OtaUpdatingState::onEnter()
{
    State::onEnter();
    lastProgress = -1;
    upgradeFinished = false;
    upgradeSuccess = false;
    lastUpdateTime = 0;
    lastFilledWidth = 0;

    context->otaInstaller->setUpdaterCallback(this);
    
    context->display->withUpdate([this](PixelDisplay& d) {
        d.selectLine(Lines::L3);
        d.print(UiTitles::Messages::OTA_INSTALLING);
    });

}

void OtaUpdatingState::onLeft()
{
    context->otaInstaller->setUpdaterCallback(nullptr);
    State::onLeft();
}

void OtaUpdatingState::buildUi(sets::Builder &b)
{
    b.LabelNum(UPT_PROGRESS, UiTitles::Updates::INSTALL_UPDATE_LABEL, 0);
}

void OtaUpdatingState::updateUi(sets::Updater &upd)
{
    upd.update(UPT_PROGRESS, context->otaInstaller->getProgress());
}

void OtaUpdatingState::printStatus(gson::Str &printer) const
{
    printer["state"] = "ota_updating";
    printer["progress"] = lastProgress;
    printer["finished"] = upgradeFinished;
    printer["success"] = upgradeSuccess;
}

const char *OtaUpdatingState::getDescription() const
{
    return "OTA Updating";
}

void OtaUpdatingState::onUpgradeStart()
{
    lastFilledWidth = 0;
    context->display->withUpdate([this](PixelDisplay& d) {
        d.clearRect(0, barY, barWidth, barHeight);
        d.selectLine(Lines::L3);
        d.print(UiTitles::Messages::OTA_INSTALLING);
    });
}

void OtaUpdatingState::onUpgradeProgress(int progress)
{
    lastProgress = progress;

    context->display->withUpdate([this, progress](PixelDisplay& d) {
        int filledWidth = (progress * barWidth) / 100;

        if (filledWidth > lastFilledWidth)
        {
            for (int x = lastFilledWidth; x < filledWidth; x++)
            {
                d.drawLine(x, barY, x, barY + barHeight - 1);
            }
        }
        else if (filledWidth < lastFilledWidth)
        {
            d.clearRect(0, barY, barWidth, barHeight);
            lastFilledWidth = 0;
            for (int x = 0; x < filledWidth; x++)
            {
                d.drawLine(x, barY, x, barY + barHeight - 1);
            }
        }

        lastFilledWidth = filledWidth;
    });
}

void OtaUpdatingState::onUpgradeEnd(const bool success)
{
    upgradeFinished = true;
    upgradeSuccess = success;
    lastProgress = success ? 100 : 0;
    lastFilledWidth = 0;

    context->display->withUpdate([this, success](PixelDisplay& d) {
        int line3Y = Lines::L3 * 8;
        d.clearRect(0, line3Y, 128, 16);
        d.selectLine(Lines::L3);
        d.print(success ? UiTitles::Messages::OTA_SUCCESS : UiTitles::Messages::OTA_FAILED);
    });
}