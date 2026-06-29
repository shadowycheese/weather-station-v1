#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_ieee802154.h"
#include "models.h"
#include "radio.h"
#include "espspi.h"

#define GPIO_MOSI 1
#define GPIO_MISO 2
#define GPIO_SCLK 3
#define GPIO_CS 4
#define GPIO_HANDSHAKE 5

extern "C"
{
    void esp_ieee802154_receive_done(uint8_t *frame, esp_ieee802154_frame_info_t *frame_info);
}

static sensor_data_t _sensor_data[256];
static volatile int16_t _sensor_data_head = 0;
static volatile int16_t _sensor_data_tail = 0;

static SemaphoreHandle_t _sensor_data_sem = NULL;

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
        memcpy(&_sensor_data[_sensor_data_head], frame + idx, sizeof(sensor_data_t));

        _sensor_data_head += 1;

        BaseType_t mainLoopTriggered = pdFALSE;

        xSemaphoreGiveFromISR(_sensor_data_sem, &mainLoopTriggered);

        if (mainLoopTriggered == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }
    }

    esp_ieee802154_receive_handle_done(frame);
}

void handleSensorData(sensor_data_t *sensorData)
{
    printf("RX: %d: %0.2f %0.2f %0.2f (Bat. %d)\n",
           sensorData->node_id,
           sensorData->reading1,
           sensorData->reading2,
           sensorData->reading3,
           sensorData->battery_mv);

    send_data_to_master(SPI2_HOST, GPIO_HANDSHAKE, (uint8_t *)sensorData, sizeof(sensor_data_t));
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    _sensor_data_sem = xSemaphoreCreateBinary();

    init_spi_slave(SPI2_HOST, GPIO_MOSI, GPIO_MISO, GPIO_SCLK, GPIO_CS, GPIO_HANDSHAKE);

    ESP_ERROR_CHECK(esp_ieee802154_enable());
    ESP_ERROR_CHECK(esp_ieee802154_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_ieee802154_set_rx_when_idle(true));
    ESP_ERROR_CHECK(esp_ieee802154_set_channel(NET_H2_SENSOR_CHANNEL));
    ESP_ERROR_CHECK(esp_ieee802154_receive());

    for (;;)
    {
        if (xSemaphoreTake(_sensor_data_sem, portMAX_DELAY) == pdTRUE)
        {
            while (_sensor_data_tail != _sensor_data_head)
            {
                handleSensorData(&_sensor_data[_sensor_data_tail]);

                _sensor_data_tail += 1;
            }
        }
    }
}
