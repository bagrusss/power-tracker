#pragma once

#include <GyverOLED.h>

#include "api/PixelDisplay.h"
#include "util/icons.h"

#define OLED_DISPLAY GyverOLED<SSD1306_128x64, OLED_NO_BUFFER>

class DisplaySSD1306 : public PixelDisplay
{
public:
    DisplaySSD1306(const uint8_t &i2cAddress = 0x3C) : PixelDisplay(128, 64),
                                                       oled(OLED_DISPLAY(i2cAddress))
    {
    }

    void init(const uint8_t &sdaPin,
              const uint8_t &sclPin) override
    {
        oled.init(sdaPin, sclPin);
        oled.clear();
        oled.update();
        oled.autoPrintln(true);
        resetDirty();
    }

    void clear() override
    {
        oled.clear();
        if (autoUpdate)
        {
            oled.update();
        }
        else
        {
            markDirty(0, 0, width - 1, height - 1);
        }
    }

    void update() override
    {
        if (autoUpdate)
        {
            oled.update();
        }
        else if (dirtyX1 >= dirtyX0 && dirtyY1 >= dirtyY0)
        {
            oled.update(dirtyX0, dirtyY0, dirtyX1, dirtyY1);
        }
        resetDirty();
    }

    void setContrast(uint8_t conrt) override
    {
        oled.setContrast(conrt);
    }

    void setCursor(uint8_t x, uint8_t y) override
    {
        oled.setCursorXY(x, y);
    }

    void moveCursorX(int pos) override
    {
        int x, y;
        oled.getCursorXY(x, y);
        oled.setCursorXY(x + pos, y);
    }

    void moveCursorY(int pos) override
    {
        int x, y;
        oled.getCursorXY(x, y);
        oled.setCursorXY(x, y + pos);
    }

    void selectLine(const Lines &line) override
    {
        oled.setCursor(0, line);
    }

    void print(const String &text, bool inv) override
    {
        print(text.c_str(), inv);
    }

    void print(const char *text, bool inv) override
    {
        int x0, y0;
        oled.getCursorXY(x0, y0);
        bool prevInv = oled.invertText(inv);
        auto size = oled.print(text) << 3;
        oled.invertText(prevInv);
        if (autoUpdate)
        {
            oled.update(x0, y0, x0 + size, y0 + 8);
        }
        else
        {
            markDirty(x0, y0, x0 + size, y0 + 8);
        }
    }

    void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) override
    {
        oled.line(x0, y0, x1, y1, OLED_STROKE);
        if (autoUpdate)
        {
            uint8_t minX = x0 < x1 ? x0 : x1;
            uint8_t maxX = x0 > x1 ? x0 : x1;
            uint8_t minY = y0 < y1 ? y0 : y1;
            uint8_t maxY = y0 > y1 ? y0 : y1;
            oled.update(minX, minY, maxX, maxY);
        }
        else
        {
            markDirty(x0, y0, x1, y1);
        }
    }

    void drawByte(uint8_t line) override
    {
        int x, y;
        oled.getCursorXY(x, y);
        oled.drawByte(line);
        if (autoUpdate)
        {
            oled.update(x, y, x + 7, y + 8);
        }
        else
        {
            markDirty(x, y, x + 7, y + 8);
        }
    }

    void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) override
    {
        oled.rect(x, y, x + w, y + h, OLED_STROKE);
        if (autoUpdate)
        {
            oled.update(x, y, x + w, y + h);
        }
        else
        {
            markDirty(x, y, x + w, y + h);
        }
    }

    void drawIcon(tracker_icons::DisplayIcon *const icon, bool inv, bool withSpace) override
    {
        if (icon)
        {
            int x0, y0;
            oled.getCursorXY(x0, y0);
            int x1 = x0 + icon->size - 1;
            int y1 = y0 + icon->size - 1;
            oled.clear(x0, y0, x1, y1);
            oled.setCursorXY(x0, y0);
            oled.drawBytes(icon->data, icon->size, inv);
            if (autoUpdate)
            {
                oled.update(x0, y0, x1, y1);
            }
            else
            {
                markDirty(x0, y0, x1, y1);
            }

            if (withSpace)
            {
                oled.setCursorXY(x0 + icon->size + 1, y0);
            }
        }
    }

    void clearRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) override
    {
        oled.clear(x, y, x + w - 1, y + h - 1);
        if (autoUpdate)
        {
            oled.update(x, y, x + w - 1, y + h - 1);
        }
        else
        {
            markDirty(x, y, x + w - 1, y + h - 1);
        }
    }

    void setAutoUpdate(bool enable) override
    {
        if (autoUpdate != enable)
        {
            if (enable)
            {
                update();
            }
            autoUpdate = enable;
            resetDirty();
        }
    }

    bool getAutoUpdate() const override
    {
        return autoUpdate;
    }

    void beginUpdate() override
    {
        setAutoUpdate(false);
    }

    void endUpdate() override
    {
        bool prev = autoUpdate;
        autoUpdate = true;
        update();
        autoUpdate = prev;
    }

    // Примеры использования паттерна строитель:
    // 1. Использование лямбды
    // withUpdate([this](PixelDisplay& d) {
    //     d.drawLine(0, 0, 10, 10);
    //     d.drawRect(20, 20, 30, 30);
    //     d.print("Hello");
    // });
    //
    // 2. Ручное управление
    // beginUpdate();
    // drawLine(...);
    // drawRect(...);
    // endUpdate();

private:
    OLED_DISPLAY oled;
    bool autoUpdate = false;

    int dirtyX0 = 0;
    int dirtyY0 = 0;
    int dirtyX1 = 0; 
    int dirtyY1 = 0;

    void markDirty(int x0, int y0, int x1, int y1)
    {
        if (dirtyX1 < dirtyX0)
        {
            dirtyX0 = x0;
            dirtyY0 = y0;
            dirtyX1 = x1;
            dirtyY1 = y1;
        }
        else
        {
            if (x0 < dirtyX0)
                dirtyX0 = x0;
            if (y0 < dirtyY0)
                dirtyY0 = y0;
            if (x1 > dirtyX1)
                dirtyX1 = x1;
            if (y1 > dirtyY1)
                dirtyY1 = y1;
        }

        if (dirtyX0 < 0)
            dirtyX0 = 0;
        if (dirtyY0 < 0)
            dirtyY0 = 0;
        if (dirtyX1 >= width)
            dirtyX1 = width - 1;
        if (dirtyY1 >= height)
            dirtyY1 = height - 1;
    }

    void resetDirty()
    {
        dirtyX0 = dirtyY0 = 0;
        dirtyX1 = dirtyY1 = -1; // инвалидная область
    }
};