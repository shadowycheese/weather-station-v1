#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "data/remote-sensor.h"
#include "ui/edt.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "data/db.h"
#include "espio.h"
#include "espi2c.h"
#include "esp_sleep.h"
#include "models.h"
#include "utils.h"
#include "log/log.h"

#define i2c_config_t bme280_i2c_config_backend_t
#include "bme280.h"
#undef i2c_config_t

#define TICK_INTERVAL 1000

#define BM280_SDA_IO GPIO_NUM_47
#define BM280_SCL_IO GPIO_NUM_27
#define BM280_I2C_NUM I2C_NUM_0
#define BM280_FREQ_HZ 100000
#define BME280_I2C_ID 0x76

#define BME280_TICKS_PER_READ 10
#define BME280_TICKS_PER_NOTIFY 10

static const char *TAG = "LOCAL-SENSOR";

i2c_bus_handle_t _bme280_i2c_bus;
bme280_handle_t _bme280_handle;

uint32_t _task_ticks = 0;

void init_bme280()
{
    app_log(LOG_INFO, TAG, "Initialized BME-280...");

    i2c_config_t config = create_i2c_input_configration(BM280_SDA_IO, BM280_SCL_IO, BM280_FREQ_HZ);

    _bme280_i2c_bus = i2c_bus_create(BM280_I2C_NUM, reinterpret_cast<const bme280_i2c_config_backend_t *>(&config));

    if (_bme280_i2c_bus == NULL)
    {
        app_log(LOG_ERROR, TAG, "Failed to create BME-280 I2C bus");
    }

    _bme280_handle = bme280_create(_bme280_i2c_bus, BME280_I2C_ID);

    if (_bme280_handle == NULL)
    {
        app_log(LOG_ERROR, TAG, "Failed to create BME-280 I2C device handle");
    }

    esp_err_t err = bme280_default_init(_bme280_handle);

    if (err == ESP_OK)
    {
        app_log(LOG_INFO, TAG, "Initialized BME-280 I2C");
    }
    else
    {
        ESP_ERROR_CHECK(err);
        app_log(LOG_ERROR, TAG, "Failed to initialized BME-280 I2C");
    }
}

void read_bme280()
{
    if ((_task_ticks % BME280_TICKS_PER_READ) != 0)
    {
        return;
    }

    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;

    ESP_ERROR_CHECK_WITHOUT_ABORT(bme280_take_forced_measurement(_bme280_handle));

    vTaskDelay(pdMS_TO_TICKS(25));

    if (bme280_read_temperature(_bme280_handle, &temperature) == ESP_OK &&
        bme280_read_humidity(_bme280_handle, &humidity) == ESP_OK &&
        bme280_read_pressure(_bme280_handle, &pressure) == ESP_OK)
    {
        sensor_data_t sensor_data;

        sensor_data.sensor_id = SENSOR_TEMP_INSIDE;
        sensor_data.reading1 = temperature;
        sensor_data.reading2 = humidity;
        sensor_data.reading3 = pressure;
        sensor_data.battery_mv = -1;

        printf("LOCAL BME: %0.2f %0.2f %0.2f\n", temperature, humidity, pressure);

        if ((_task_ticks % BME280_TICKS_PER_NOTIFY) != 0)
        {
            Db::handle_sensor_data(sensor_data);

            // edt_post(job);
        }
    }
    else
    {
        ESP_LOGW(TAG, "Failed to read data from BME280.. retrying");

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void read_sensors(void *)
{
    const TickType_t delay_period = pdMS_TO_TICKS(TICK_INTERVAL);

    TickType_t last_wake_time = xTaskGetTickCount();

    ESP_LOGI(TAG, "Collect sensor data...");

    for (;;)
    {
        read_bme280();

        vTaskDelayUntil(&last_wake_time, delay_period);

        _task_ticks++;
    }
}

extern "C"
{
    void local_sensor_read_start()
    {
        init_bme280();

        app_log(LOG_INFO, TAG, "Starting local sensor read task");
        BaseType_t result = xTaskCreatePinnedToCore(
            read_sensors,
            "local_sensors",
            4096,
            NULL,
            5,
            NULL,
            0);

        if (result != pdTRUE)
        {
            ESP_LOGE(TAG, "Failed to create FreeRTOS task!");
        }
    }
}