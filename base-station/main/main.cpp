#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_touch.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
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
#include "net/forecast-task.h"
#include "utils/json.h"

const char *TAG = "APP";

Display *_display;
MetricsRepository *_metrics_repo;

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Memory PSRAM Free: %ld, Largest: %ld | INTERNAL Free: %ld, Largest: %ld",
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
             heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));

    nvs_flash_init();

    ESP_LOGI(TAG, "Starting File systems...");

    ESP_LOGI(TAG, "File system started");

    json_init();

    edt_init();

    _display = new Display();
    _metrics_repo = new MetricsRepository();

    _display->start();
    _metrics_repo->init();

    wifi_init();

    app_log(LOG_INFO, TAG, "Starting GPIO ISR service");

    gpio_install_isr_service(0);

    remote_sensor_read_start();

    forecast_init();

    _display->set_boot_complete();
}
