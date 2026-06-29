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

    create_cpp20_ui();

    bsp_display_unlock();
}

lv::Button *my_button_ptr = nullptr;

void Display::create_cpp20_ui()
{
    auto active_screen = lv::screen_active();

    // Allocate on the heap so the event handler stays alive in memory
    my_button_ptr =
        new lv::Button(lv::Button::create(active_screen)
                           .size(150, 100)
                           .center()
                           .on(LV_EVENT_CLICKED,
                               [](lv_event_t *e)
                               {
                                   printf("Button clicked! Triggering action "
                                          "inside C++ lambda...\n");
                               }));

    _label = lv::Label::create(*my_button_ptr).text("Click Me").center();
}

void Display::setText(char *text)
{
    _label.text(text);

    _label.invalidate();
}
