#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/display.h"
#include "display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "lv.hpp"
#include "lv_demos.h"
#include "lvgl.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "models.h"
#include "edt.h"

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

    //_parent = lv_screen_active();

    bsp_display_lock(-1);

    init_ui();

    bsp_display_unlock();
}

void Display::init_ui()
{
    auto active_screen = lv::screen_active();

    // Allocate on the heap so the event handler stays alive in memory
    auto root = lv::vbox(active_screen)
                    .bg_color(lv_color_black())
                    .fill()           // Fill the entire parent screen
                    .center_content() // Center items horizontally and vertically
                    .gap(20);         // Add 20px of vertical space between each label

    _logMessages = lv::Textarea::create(root)
                       .width(720)
                       .height(1280)
                       .align(LV_ALIGN_TOP_LEFT);

    // 2. Add 4 large labels to the root layout container
    // Note: LVGL 9 utilizes the default system font scales (e.g., large/large)
    _labelTemp = lv::Label::create(root)
                     .text("Sensor Reading 1")
                     .font(&lv_font_montserrat_26) // Use a large built-in font size
                     .center()
                     .visible(false);

    _labelHumidity = lv::Label::create(root)
                         .text("Sensor Reading 2")
                         .font(&lv_font_montserrat_26)
                         .center()
                         .visible(false);

    _labelPressure = lv::Label::create(root)
                         .text("Sensor Reading 3")
                         .font(&lv_font_montserrat_26)
                         .center()
                         .visible(false);

    _labelUV = lv::Label::create(root)
                   .text("Battery Level")
                   .font(&lv_font_montserrat_26)
                   .center()
                   .visible(false);
}

void Display::set_boot_complete()
{
    _logMessages.visible(false);
    _labelTemp.visible(true);
    _labelHumidity.visible(true);
    _labelPressure.visible(true);
    _labelUV.visible(true);
}

void Display::log_message(log_level_t log_level, const char *message)
{
    bsp_display_lock(-1);

    _instance->_logMessages.add_text(message);
    _instance->_logMessages.add_text("\n");

    bsp_display_unlock();
}

void Display::setValues(char *temp, char *hum, char *pres, char *uv)
{
    if (temp)
    {
        _labelTemp.text(temp);
    }
    if (hum)
    {
        _labelHumidity.text(hum);
    }
    if (pres)
    {
        _labelPressure.text(pres);
    }
    if (uv)
    {
        _labelUV.text(uv);
    }
}
