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
    lv::Flex root = create_box(parent, SCREEN_WIDTH, 80);

    _labelTime = lv::Label::create(root)
                     .font(FONT_24PT)
                     .text_color(COL_TEXT_PRIMARY);

    _labelDate = lv::Label::create(root)
                     .font(FONT_24PT)
                     .text_color(COL_TEXT_SECONDARY);

    edt_add_system_event_handler(SYSTEM_EVENT_TICK, [this](system_event_t m)
                                 {
                                     char ts[10], ds[24];

                                     time_t t = time(NULL);

                                     struct tm timeinfo;

                                     localtime_r(&t, &timeinfo);

                                     strftime(ts, 10, "%H:%M:%S", &timeinfo);
                                     strftime(ds, 12, "%A, %d %s %Y", &timeinfo);

                                     _labelDate.text(ds);
                                     _labelTime.text(ts); //
                                 });

    _labelWifi = lv::Label::create(root)
                     .text("WIFI")
                     .font(FONT_24PT)
                     .text_color(COL_TEXT_SECONDARY);
    _labelStatus = lv::Label::create(root)
                       .text("STATUS")
                       .font(FONT_24PT) // Use a large built-in font size
                       .text_color(COL_TEXT_SECONDARY);
};
