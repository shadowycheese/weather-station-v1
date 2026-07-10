#include "display.h"
#include "bsp/esp-bsp.h"
#include "lv.hpp"
#include "lvgl.h"
#include "models.h"
#include "ui/ui-helpers.h"
#include "ui/fonts/fonts.h"

Display *Display::_instance = NULL;

Display::Display()
{
    _instance = this;
}

void Display::start()
{
    static bsp_display_cfg_t cfg = {
        .lv_adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG(),
        .rotation = ESP_LV_ADAPTER_ROTATE_0,
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_TRIPLE_PARTIAL,
        .touch_flags = {.swap_xy = 0, .mirror_x = 0, .mirror_y = 0}};

    bsp_display_start_with_config(&cfg);

    bsp_display_backlight_on();

    bsp_display_lock(-1);

    init_fonts();

    init_ui();

    bsp_display_unlock();
}

void Display::init_ui()
{
    _logMessages = lv::Textarea::create(NULL)
                       .width(SCREEN_WIDTH)
                       .height(SCREEN_HEIGHT)
                       .font(&lv_font_montserrat_22)
                       .align(LV_ALIGN_TOP_LEFT);

    lv_screen_load(_logMessages);

    _mainPanel = create_vbox(NULL, SCREEN_WIDTH, SCREEN_HEIGHT).outline_width(0);

    for (int i = 0; i < 3; i++)
    {
        _cards[i]->init(_mainPanel);
    }

    create_vbox(_mainPanel, SCREEN_WIDTH, 0).fill().bg_color(COL_CYAN);
}

void Display::set_boot_complete()
{
    bsp_display_lock(-1);

    lv_screen_load(_mainPanel);

    bsp_display_unlock();
}

void Display::log_message(log_level_t log_level, const char *message)
{
    bsp_display_lock(-1);

    _instance->_logMessages.add_text(message);
    _instance->_logMessages.add_text("\n");

    bsp_display_unlock();
}
