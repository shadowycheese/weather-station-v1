#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "models.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include "espio.h"
#include "radio.h"
#include "utils.h"

#define RAIN_TIP_PIN GPIO_NUM_7

#define DEEP_SLEEP_MILLIS (60ULL * 1000ULL * 1000ULL)

static const char *TAG = "MAIN_APP";

void enter_deep_sleep(uint64_t us)
{
    esp_sleep_enable_ext1_wakeup_io(BIT(GPIO_NUM_10), ESP_EXT1_WAKEUP_ANY_HIGH);
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

    // enter_deep_sleep(ERROR_DEEP_SLEEP_MILLIS);
}

void handle_error(const char *msg)
{
    ESP_LOGE(TAG, "%s", msg);

    // enter_deep_sleep(ERROR_DEEP_SLEEP_MILLIS);
}

extern "C" void app_main(void)
{
    ensure_success(nvs_flash_init());

    /*

    // ensure_success(configure_output_pin(UV_POWER_PIN));

    // ensure_success(gpio_set_level(UV_POWER_PIN, 1));

    vTaskDelay(pdMS_TO_TICKS(10));

    adc_oneshot_unit_handle_t adc1_handle;

    adc_oneshot_unit_init_cfg_t init_config1 = {};
    init_config1.unit_id = ADC_UNIT_1;
    init_config1.ulp_mode = ADC_ULP_MODE_DISABLE;

    ensure_success(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {};
    // config.bitwidth = ADC_BITWIDTH_DEFAULT; // 12-bit for ESP32-H2
    // config.atten = UV_ADC_ATTEN;            // Up to 3.3V range

    // ensure_success(adc_oneshot_config_channel(adc1_handle, UV_ADC_CHANNEL, &config));

    // ----------------------------------------------------
    // 4. Initialize Calibration (Line Fitting)
    // ----------------------------------------------------
    adc_cali_handle_t cali_handle = NULL;

    adc_cali_curve_fitting_config_t cali_config = {};
    cali_config.unit_id = ADC_UNIT_1;
    // cali_config.atten = UV_ADC_ATTEN;
    cali_config.bitwidth = ADC_BITWIDTH_DEFAULT;

   // bool requiresCalibration = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle) != ESP_OK;

    //int raw_value;
    //int voltage_mv;

    //ensure_success(adc_oneshot_read(adc1_handle, UV_ADC_CHANNEL, &raw_value));

    float uv;

    // Convert to calibrated voltage (mV) using ESP-IDF factory data
    if (requiresCalibration)
    {
        ensure_success(adc_cali_raw_to_voltage(cali_handle, raw_value, &voltage_mv));

        uv = (float)voltage_mv;
    }
    else
    {
        uv = (float)raw_value;
    }*/

    sensor_data_t sensor_data;

    sensor_data.sensor_id = SENSOR_OUTSIDE_UV;
    sensor_data.reading1 = 1;
    sensor_data.reading2 = 0;
    sensor_data.reading3 = 0;
    sensor_data.battery_mv = 0;

    ieee_802154_transmit_sensor_data(SENSOR_OUTSIDE_UV, &sensor_data);

    vTaskDelay(pdMS_TO_TICKS(20));

    // gpio_set_level(UV_POWER_PIN, 0);

    // enter_deep_sleep(DEEP_SLEEP_MILLIS);
}
