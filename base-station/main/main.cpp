#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_touch.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "espio.h"
#include "espspi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_demos.h"
#include "lvgl.h"
#include "models.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "ui/display.h"
#include "ui/edt.h"
#include "io/sensor.h"

Display display;

extern "C" void app_main(void)
{
    gpio_install_isr_service(0);

    edt_init(&display);

    display.start();

    sensor_read_start();
}
