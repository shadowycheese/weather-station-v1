#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "models.h"
#include "nvs_flash.h"
#include "espadc.h"
#include "espio.h"
#include "radio.h"
#include "utils.h"

#define UV_POWER_PIN GPIO_NUM_4
#define UV_DATA_PIN GPIO_NUM_1
#define UV_ADC_CHANNEL ADC_CHANNEL_0
#define BATTERY_PIN GPIO_NUM_5
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_4

#define DEEP_SLEEP_MILLIS (60ULL * 1000ULL * 1000ULL)
#define ERROR_DEEP_SLEEP_MILLIS (5ULL * 1000ULL * 1000ULL)

static const char *TAG = "MAIN_APP";

void enter_deep_sleep(uint64_t us)
{
    esp_sleep_enable_timer_wakeup(us);

    esp_deep_sleep_start();
}

void ensure_success(esp_err_t err)
{
    if (err == ESP_OK)
    {
        return;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    enter_deep_sleep(ERROR_DEEP_SLEEP_MILLIS);
}

void handle_error(const char *msg)
{
    ESP_LOGE(TAG, "%s", msg);

    enter_deep_sleep(ERROR_DEEP_SLEEP_MILLIS);
}

extern "C" void app_main(void)
{
    ensure_success(nvs_flash_init());

    ensure_success(configure_output_pin(UV_POWER_PIN));

    ensure_success(gpio_set_level(UV_POWER_PIN, 1));

    vTaskDelay(pdMS_TO_TICKS(10));

    int uv_mv = read_adc(UV_ADC_CHANNEL);
    float uv = (float)uv_mv;

    int16_t battery = (read_adc(BATTERY_ADC_CHANNEL) & 0xFFFF) * 2;

    sensor_data_t sensor_data;

    sensor_data.sensor_id = SENSOR_OUTSIDE_UV;
    sensor_data.reading1 = uv;
    sensor_data.reading2 = 0;
    sensor_data.reading3 = 0;
    sensor_data.battery_mv = battery;

    ieee_802154_transmit_sensor_data(SENSOR_OUTSIDE_UV, &sensor_data);

    vTaskDelay(pdMS_TO_TICKS(20));

    disable_pin(BATTERY_PIN);
    disable_pin(UV_DATA_PIN);

    gpio_set_level(UV_POWER_PIN, 0);

    enter_deep_sleep(DEEP_SLEEP_MILLIS);
}
