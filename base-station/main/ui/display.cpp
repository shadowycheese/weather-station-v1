#include "bsp/display.h"
#include "display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv.hpp"
#include "lv_demos.h"
#include "lvgl.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_sntp.h"

void Display::start()
{
    static bsp_display_cfg_t cfg = {
        .lv_adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG(),
        .rotation = ESP_LV_ADAPTER_ROTATE_90,
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_TRIPLE_PARTIAL,
        .touch_flags = {.swap_xy = 1, .mirror_x = 0, .mirror_y = 0}};

    bsp_display_start_with_config(&cfg);

    bsp_display_backlight_on();

    //_parent = lv_screen_active();

    bsp_display_lock(-1);

    create_boot_io();

    bsp_display_unlock();
}

lv::Button *my_button_ptr = nullptr;

void Display::create_boot_io()
{
    auto active_screen = lv::screen_active();

    // Allocate on the heap so the event handler stays alive in memory
    auto root = lv::vbox(active_screen)
                    .fill()           // Fill the entire parent screen
                    .center_content() // Center items horizontally and vertically
                    .gap(20);         // Add 20px of vertical space between each label

    // 2. Add 4 large labels to the root layout container
    // Note: LVGL 9 utilizes the default system font scales (e.g., large/large)
    _labelTemp = lv::Label::create(root)
                     .text("Sensor Reading 1")
                     .font(&lv_font_montserrat_26) // Use a large built-in font size
                     .center();

    _labelHumidity = lv::Label::create(root)
                         .text("Sensor Reading 2")
                         .font(&lv_font_montserrat_26)
                         .center();

    _labelPressure = lv::Label::create(root)
                         .text("Sensor Reading 3")
                         .font(&lv_font_montserrat_26)
                         .center();

    _labelUV = lv::Label::create(root)
                   .text("Battery Level")
                   .font(&lv_font_montserrat_26)
                   .center();
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
