#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "models.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include "espio.h"
#include "espi2c.h"
#include "radio.h"
#include "utils.h"

#define i2c_config_t bme280_i2c_config_backend_t
#include "bme280.h"
#undef i2c_config_t

#define I2C_MASTER_SDA_IO GPIO_NUM_4
#define I2C_MASTER_SCL_IO GPIO_NUM_5
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 10000

#define BME280_I2C_ADDRESS 0x77
#define BME280_POWER_PIN GPIO_NUM_2

#define DEEP_SLEEP_MILLIS (10ULL * 1000ULL * 1000ULL)
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

    ensure_success(configure_output_pin(BME280_POWER_PIN));

    ensure_success(gpio_set_level(BME280_POWER_PIN, 1));

    vTaskDelay(pdMS_TO_TICKS(10));

    i2c_config_t config = create_i2c_input_configration(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ);

    i2c_bus_handle_t i2c_bus = i2c_bus_create(I2C_MASTER_NUM, reinterpret_cast<const bme280_i2c_config_backend_t *>(&config));

    if (i2c_bus == NULL)
    {
        handle_error("BME280 I2C bus creation failed");
    }

    bme280_handle_t bme280 = bme280_create(i2c_bus, 0x77);

    if (bme280 == NULL)
    {
        handle_error("BME280 device creation failed");
    }

    ensure_success(bme280_default_init(bme280));

    ensure_success(bme280_take_forced_measurement(bme280));

    vTaskDelay(pdMS_TO_TICKS(25));

    sensor_data_t sensor_data;

    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;

    sensor_data.sensor_id = SENSOR_TEMP_OUTSIDE1;

    for (;;)
    {
        if (bme280_read_temperature(bme280, &temperature) == ESP_OK &&
            bme280_read_humidity(bme280, &humidity) == ESP_OK &&
            bme280_read_pressure(bme280, &pressure) == ESP_OK)
        {
            sensor_data.reading1 = temperature;
            sensor_data.reading2 = humidity;
            sensor_data.reading3 = pressure;

            ieee_802154_transmit_sensor_data(SENSOR_TEMP_OUTSIDE1, &sensor_data);

            vTaskDelay(pdMS_TO_TICKS(20));

            break;
        }
        else
        {
            ESP_LOGW(TAG, "Failed to read data from BME280.. retrying");

            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    gpio_set_level(BME280_POWER_PIN, 0);

    i2c_driver_delete(I2C_MASTER_NUM);

    gpio_set_direction(I2C_MASTER_SDA_IO, GPIO_MODE_INPUT);
    gpio_set_direction(I2C_MASTER_SCL_IO, GPIO_MODE_INPUT);

    enter_deep_sleep(DEEP_SLEEP_MILLIS);
}
