#pragma once

#include <util/icons.h>

enum Lines
{
    L0,
    L1,
    L2,
    L3,
    L4,
    L5,
    L6,
    L7,

    Lines_SIZE
};

class PixelDisplay
{
public:
    PixelDisplay(uint16_t w, uint16_t h) : width(w), height(h) {}
    virtual ~PixelDisplay() = default;

    const uint16_t width;
    const uint16_t height;

    virtual void init(const uint8_t &sdaPin,
                      const uint8_t &sclPin) = 0;
    virtual void clear() = 0;
    virtual void clearRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) = 0;
    virtual void update() = 0;

    virtual void setContrast(uint8_t conrt) = 0;

    virtual void setCursor(const uint8_t x, uint8_t y) = 0;
    virtual void moveCursorX(int pos) = 0;
    virtual void moveCursorY(int pos) = 0;
    virtual void selectLine(const Lines &line) = 0;
    virtual void print(const String &text, bool inv = false) = 0;
    virtual void print(const char *text, bool inv = false) = 0;

    virtual void drawByte(uint8_t line) = 0;
    virtual void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) = 0;
    virtual void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) = 0;

    virtual void drawIcon(tracker_icons::DisplayIcon *const icon, bool inv = false, bool withSpace = true) = 0;

    virtual void setAutoUpdate(bool enable) { }
    virtual bool getAutoUpdate() const { return true; }
    virtual void beginUpdate() { setAutoUpdate(false); }
    virtual void endUpdate() { setAutoUpdate(true); update(); }

    template<typename F>
    void withUpdate(F&& func)
    {
        beginUpdate();
        func(*this);
        endUpdate();
    }
};