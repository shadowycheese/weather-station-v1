#include "lv.hpp"
#include "lvgl.h"
#include "ui/core/card.h"
#include "ui/header.h"
#include "ui/theme.h"
#include "ui/ui-helpers.h"
#include "events/edt.h"

Header::Header()
{
}

void Header::init(lv::ObjectView parent)
{
    lv::Flex root = create_hbox(parent, SCREEN_WIDTH, 50)
                        .bg_color(COL_BACKGROUND_2)
                        .border_width(1)
                        .outline_width(1)
                        .pad_top(6)
                        .pad_left(6)
                        .pad_right(6);

    _labelDate = lv::Label::create(root)
                     .margin_top(-3)
                     .grow(1)
                     .font(FONT_24PT_BOLD)
                     .text_color(COL_TEXT_SECONDARY);
    _labelTime = lv::Label::create(root)
                     .font(FONT_24PT)
                     .margin(-3)
                     .text_color(COL_TEXT_SECONDARY);

    _labelWifi = lv::Label::create(root)
                     .padding(3)
                     .text("\uF09E")
                     .font(FONT_SYMBOL_24PT)
                     .margin(-3)
                     .text_color(COL_TEXT_SECONDARY);

    edt_add_system_event_handler(SYSTEM_EVENT_WIFI_STATUS, [this](system_event_t m)
                                 {
                                     _labelWifi.text_color(m.value ? m.value < 0 ? COL_TEXT_ERROR : COL_TEXT_SECONDARY : COL_TEXT_MUTED); //
                                 });

    edt_add_system_event_handler(SYSTEM_EVENT_TICK, [this](system_event_t m)
                                 {
                                     char ts[10], ds[32];

                                     time_t t = time(NULL);

                                     t += TIME_ZONE_OFFSET;

                                     struct tm timeinfo;

                                     localtime_r(&t, &timeinfo);

                                     strftime(ts, 10, "%H:%M:%S", &timeinfo);
                                     strftime(ds, 32, "%A, %d %b %Y", &timeinfo);

                                     _labelDate.text(ds);
                                     _labelTime.text(ts); //
                                 });
};
