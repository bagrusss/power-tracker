#pragma once

namespace tracker_icons
{

    struct DisplayIcon
    {
        uint8_t data[8];
        const uint8_t size;
    };

    const DisplayIcon led_icons[] = {
        {{0x02, 0x09, 0x25, 0x95, 0x95, 0x25, 0x09, 0x02}, 8}, // WIFI
        {{0x18, 0x7e, 0x7e, 0xe7, 0xe7, 0x7e, 0x7e, 0x18}, 8}, // PREFS
        {{0x00, 0x98, 0xdc, 0xfe, 0x7f, 0x3b, 0x19, 0x18}, 8}, // FLASH
        {{0x18, 0x3c, 0x7e, 0xff, 0x7e, 0x3c, 0x18, 0x00}, 8}, // OTA (arrow up)
    };

    const uint8_t led_icons_count = sizeof(led_icons) / sizeof(DisplayIcon);

#define WIFI led_icons[0]
#define PREFS led_icons[1]
#define FLASH led_icons[2]
#define OTA led_icons[3]

}