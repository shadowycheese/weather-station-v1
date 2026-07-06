#include "lv.hpp"
#include "lvgl.h"
#include "ui/core/card.h"
#include "ui/header.h"
#include "ui/theme.h"
#include "ui/ui-helpers.h"

Header::Header()
{
}

void Header::init(lv::ObjectView parent)
{
    lv::Flex root = create_box(parent, SCREEN_WIDTH, 80);

    _labelTime = lv::Label::create(root)
                     .text("00:00:00")
                     .font(FONT_24PT)
                     .text_color(COL_TEXT_PRIMARY);

    _labelDate = lv::Label::create(root)
                     .text("03 JUL 2026")
                     .font(FONT_24PT)
                     .text_color(COL_TEXT_SECONDARY);

    _labelWifi = lv::Label::create(root)
                     .text("WIFI")
                     .font(FONT_24PT)
                     .text_color(COL_TEXT_SECONDARY);
    _labelStatus = lv::Label::create(root)
                       .text("STATUS")
                       .font(FONT_24PT) // Use a large built-in font size
                       .text_color(COL_TEXT_SECONDARY);
};
