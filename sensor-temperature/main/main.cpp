#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "models.h"
#include "nvs_flash.h"
#include "driver/i2c_master.h"
#include "espio.h"
#include "radio.h"
#include "utils.h"
#include "bme280.h"

#define I2C_MASTER_SDA_IO GPIO_NUM_4
#define I2C_MASTER_SCL_IO GPIO_NUM_5
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

#define BME280_I2C_ADDRESS 0x76
#define BME280_POWER_PIN GPIO_NUM_3

#define DEEP_SLEEP_MILLIS (60ULL * 1000ULL * 1000ULL)
#define ERROR_DEEP_SLEEP_MILLIS (30ULL * 1000ULL * 1000ULL)

static const char *TAG = "MAIN_APP";

static i2c_master_bus_handle_t _i2c_bus;

void enter_deep_sleep(uint64_t us)
{
    esp_sleep_pd_config(ESP_PD_DOMAIN_RC_FAST, ESP_PD_OPTION_OFF);

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

    ensure_success(gpio_hold_dis((gpio_num_t)BME280_POWER_PIN));

    ensure_success(configure_output_pin(BME280_POWER_PIN));

    ensure_success(gpio_set_level(BME280_POWER_PIN, 1));

    vTaskDelay(pdMS_TO_TICKS(10));

    i2c_master_bus_config_t bus_config = {};
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.i2c_port = I2C_MASTER_NUM;
    bus_config.sda_io_num = I2C_MASTER_SDA_IO;
    bus_config.scl_io_num = I2C_MASTER_SCL_IO;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;
    ensure_success(i2c_new_master_bus(&bus_config, &_i2c_bus));

    ensure_success(bme280_init(_i2c_bus, BME280_I2C_ADDRESS, I2C_MASTER_FREQ_HZ, true));

    vTaskDelay(pdMS_TO_TICKS(30));

    sensor_data_t sensor_data;

    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;

    sensor_data.sensor_id = SENSOR_TEMP_OUTSIDE1;

    for (;;)
    {
        if (bme280_readout(&temperature, &pressure, &humidity) == ESP_OK)
        {
            sensor_data.reading1 = temperature;
            sensor_data.reading2 = humidity;
            sensor_data.reading3 = pressure;
            sensor_data.battery_mv = -1;

            ieee_802154_transmit_sensor_data(SENSOR_TEMP_OUTSIDE1, &sensor_data);

            // Give the radio some time
            vTaskDelay(pdMS_TO_TICKS(20));

            break;
        }
        else
        {
            ESP_LOGW(TAG, "Failed to read data from BME280.. retrying");
        }
    }

    bme280_delete();

    i2c_del_master_bus(_i2c_bus);

    gpio_set_level(BME280_POWER_PIN, 0);

    gpio_pullup_dis(BME280_POWER_PIN);
    gpio_pulldown_dis(BME280_POWER_PIN);

    gpio_set_direction(I2C_MASTER_SDA_IO, GPIO_MODE_INPUT);
    gpio_set_direction(I2C_MASTER_SCL_IO, GPIO_MODE_INPUT);

    gpio_hold_en((gpio_num_t)BME280_POWER_PIN);

    enter_deep_sleep(DEEP_SLEEP_MILLIS);
}
