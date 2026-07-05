#include "lv.hpp"
#include "lvgl.h"
#include "ui/core/card.h"
#include "header.h"

void Header::init(lv::ObjectView parent)
{
    // Allocate on the heap so the event handler stays alive in memory
    auto root = lv::vbox(parent)
                    .bg_color(lv_color_black())
                    .border_width(0)
                    .outline_width(0)
                    .radius(0)
                    .gap(20)
                    .height(80)
                    .width(720);

    _labelTime = lv::Label::create(root)
                     .text("00:00:00")
                     .font(&lv_font_montserrat_26) // Use a large built-in font size
                     .text_color(lv_color_white());

    _labelDate = lv::Label::create(root)
                     .text("03 JUL 2026")
                     .font(&lv_font_montserrat_24) // Use a large built-in font size
                     .text_color(lv_color_white());

    _labelWifi = lv::Label::create(root)
                     .text("WIFI")
                     .font(&lv_font_montserrat_24) // Use a large built-in font size
                     .text_color(lv_color_white());
    _labelStatus = lv::Label::create(root)
                       .text("STATUS")
                       .font(&lv_font_montserrat_24) // Use a large built-in font size
                       .text_color(lv_color_white());
};
