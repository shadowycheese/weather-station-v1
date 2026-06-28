#include "bsp/esp-bsp.h"
#include "lvgl.h"

extern "C" void app_main(void)
{
    // Init full board (DSI + power + touch + LVGL)
    bsp_display_start();

    // Backlight ON
    bsp_display_backlight_on();

    // Get LVGL screen
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello World!");
    lv_obj_center(label);

    while (true)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}