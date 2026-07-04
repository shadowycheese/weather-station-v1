#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_ieee802154.h"
#include "esp_log.h"
#include "models.h"
#include "espuart.h"
#include "radio.h"
#include "espio.h"
#include "sgp30/sgp30.h"
#include "bme280/bme280.h"
#include "math.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"

#define UART_PORT UART_NUM_1
#define UART_RX_PIN 0  // YELLOW -> 20
#define UART_TX_PIN 1  // ORANGE -> 21
#define UART_RTS_PIN 2 // BLUE -> 22n
#define UART_BUF_SIZE 256
#define UART_SPEED 460800

#define TICK_INTERVAL 1000
#define BME280_TICKS_PER_SEND 1
#define SGP30_TICKS_BEFORE_READ 15
#define SGP30_TICKS_PER_SEND 1

#define I2C_SDA_IO GPIO_NUM_4
#define I2C_SCL_IO GPIO_NUM_5
#define I2C_NUM I2C_NUM_0
#define I2C_FREQ_HZ 100000

#define BME280_I2C_ID (0x77)
#define SGP30_I2C_ID (0x58)

static portMUX_TYPE _buffer_mux = portMUX_INITIALIZER_UNLOCKED;

static const char *TAG = "RECEIVER";

uint32_t _task_ticks = 0;

extern "C"
{
    void esp_ieee802154_receive_done(uint8_t *frame, esp_ieee802154_frame_info_t *frame_info);
}

static i2c_master_bus_handle_t _i2c_bus;
static bmx280_t *_bmx280;

static sensor_data_t _sensor_data[256];
static volatile uint8_t _sensor_data_head = 0;
static volatile uint8_t _sensor_data_tail = 0;

static SemaphoreHandle_t _sensor_data_sem = NULL;

inline void queue_sensor_data(void *sensor_data)
{
    taskENTER_CRITICAL(&_buffer_mux);
    memcpy(&_sensor_data[_sensor_data_head], sensor_data, sizeof(sensor_data_t));

    _sensor_data_head += 1;
    taskEXIT_CRITICAL(&_buffer_mux);
}

void esp_ieee802154_receive_done(uint8_t *frame, esp_ieee802154_frame_info_t *frame_info)
{
    int idx = 0;

    int8_t len = frame[idx++];
    int invalid = 0;

    if (len < (sizeof(sensor_data_t) + 8))
    {
        invalid = 1;
    }

    if (frame[idx++] != 0x01)
    {
        invalid = 2;
    }

    if (frame[idx++] != 0x08)
    {
        invalid = 3;
    }

    idx++; // Skip seq

    uint16_t pan_id = frame[idx++];

    pan_id &= (frame[idx++] << 8);

    if (pan_id != NET_H2_SENSOR_PAN_ID)
    {
        invalid = pan_id;
    }

    idx += 2; // Skip sensor id

    if (invalid == 0)
    {
        queue_sensor_data(frame + idx);

        BaseType_t mainLoopTriggered = pdFALSE;

        xSemaphoreGiveFromISR(_sensor_data_sem, &mainLoopTriggered);

        if (mainLoopTriggered == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }
    }

    esp_ieee802154_receive_handle_done(frame);
}

void handle_sensor_data(sensor_data_t *sensor_data)
{
    printf("Sending: %d: %0.2f %0.2f %0.2f (Bat. %d)\n",
           sensor_data->sensor_id,
           sensor_data->reading1,
           sensor_data->reading2,
           sensor_data->reading3,
           sensor_data->battery_mv);

    uart_write_bytes(UART_PORT, sensor_data, sizeof(sensor_data_t));

    vTaskDelay(pdMS_TO_TICKS(20));
}

void read_sgp30()
{
    if (_task_ticks < SGP30_TICKS_BEFORE_READ)
    {
        return;
    }

    sensor_data_t data = {};
    data.battery_mv = 0;
    data.sensor_id = SENSOR_AQ_INSIDE;

    uint16_t co2 = 0;
    uint16_t tvoc = 0;
    if (sgp30_read_measurements(&co2, &tvoc) == ESP_OK)
    {
        if ((_task_ticks % SGP30_TICKS_PER_SEND) != 0)
        {
            return;
        }

        data.reading1 = (float)co2;
        data.reading2 = (float)tvoc;

        queue_sensor_data(&data);

        xSemaphoreGive(_sensor_data_sem);
    }
}

float calculate_absolute_humidity(float temperature, float relative_humidity)
{
    // Bound check incoming relative humidity to stay within safe physical bounds
    if (relative_humidity > 100.0f)
    {
        relative_humidity = 100.0f;
    }
    if (relative_humidity < 0.0f)
    {
        relative_humidity = 0.0f;
    }

    float numerator = (relative_humidity / 100.0f) * 6.112f * expf((17.62f * temperature) / (243.12f + temperature));
    float denominator = 273.15f + temperature;

    // Returns absolute humidity in grams per cubic meter (g/m³)
    return 216.7f * (numerator / denominator);
}

void read_bme280()
{
    if ((_task_ticks % BME280_TICKS_PER_SEND) != 0)
    {
        return;
    }

    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;

    if (bmx280_readout(_bmx280, &temperature, &pressure, &humidity) == ESP_OK)
    {
        sensor_data_t sensor_data;

        sensor_data.sensor_id = SENSOR_TEMP_INSIDE;
        sensor_data.reading1 = temperature;
        sensor_data.reading2 = humidity;
        sensor_data.reading3 = pressure;
        sensor_data.battery_mv = -1;

        queue_sensor_data(&sensor_data);

        float absolute_humidity = calculate_absolute_humidity(temperature, humidity);

        ESP_ERROR_CHECK_WITHOUT_ABORT(sgp30_set_humidity(absolute_humidity));

        xSemaphoreGive(_sensor_data_sem);
    }
    else
    {
        ESP_LOGW(TAG, "Failed to read data from BME280");

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
        read_sgp30();
        read_bme280();

        vTaskDelayUntil(&last_wake_time, delay_period);

        _task_ticks++;
    }
}

void sensor_read_start()
{
    i2c_master_bus_config_t bus_config = {};
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.i2c_port = I2C_NUM;
    bus_config.sda_io_num = I2C_SDA_IO;
    bus_config.scl_io_num = I2C_SCL_IO;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &_i2c_bus));

    sgp30_init(_i2c_bus, SGP30_I2C_ID, I2C_FREQ_HZ);
    _bmx280 = bmx280_create_master(_i2c_bus);

    bmx280_device_create(_bmx280, BME280_I2C_ID);
    bmx280_init(_bmx280);

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

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    _sensor_data_sem = xSemaphoreCreateBinary();

    ESP_ERROR_CHECK(start_uart(UART_PORT,
                               UART_TX_PIN,
                               UART_RX_PIN,
                               UART_RTS_PIN,
                               UART_PIN_NO_CHANGE,
                               UART_BUF_SIZE,
                               UART_SPEED));

    ESP_ERROR_CHECK(esp_ieee802154_enable());
    ESP_ERROR_CHECK(esp_ieee802154_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_ieee802154_set_rx_when_idle(true));
    ESP_ERROR_CHECK(esp_ieee802154_set_channel(NET_H2_SENSOR_CHANNEL));
    ESP_ERROR_CHECK(esp_ieee802154_receive());

    sensor_read_start();

    for (;;)
    {
        if (xSemaphoreTake(_sensor_data_sem, portMAX_DELAY) == pdTRUE)
        {
            while (_sensor_data_tail != _sensor_data_head)
            {
                handle_sensor_data(&_sensor_data[_sensor_data_tail]);

                _sensor_data_tail += 1;
            }
        }
    }
}
