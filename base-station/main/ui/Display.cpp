#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_demos.h"
#include "lvgl.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "Display.h"

void Display::start()
{
    static bsp_display_cfg_t cfg = {
        .lv_adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG(),
        .rotation = ESP_LV_ADAPTER_ROTATE_90,
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_TRIPLE_PARTIAL,
        .touch_flags = {.swap_xy = 0, .mirror_x = 0, .mirror_y = 0}};

    bsp_display_start_with_config(&cfg);

    bsp_display_backlight_on();

    _parent = lv_screen_active();
    bsp_display_lock(-1);

    lv_demo_widgets();

    bsp_display_unlock();
}
