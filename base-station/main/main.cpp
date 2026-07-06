#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_touch.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "espio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_demos.h"
#include "lvgl.h"
#include "models.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "ui/display.h"
#include "events/edt.h"
#include "metrics/remote-sensor.h"
#include "metrics/metrics-repo.h"
#include "net/network.h"
#include "log/log.h"

const char *TAG = "APP";

Display display;
MetricsRepository metrics_repo;

extern "C" void app_main(void)
{
    nvs_flash_init();

    edt_init();

    display.start();

    // wifi_init();

    app_log(LOG_INFO, TAG, "Starting GPIO ISR service");
    gpio_install_isr_service(0);

    remote_sensor_read_start();

    display.set_boot_complete();
}
