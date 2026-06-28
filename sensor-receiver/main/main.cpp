#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_ieee802154.h"
#include "models.h"

extern "C"
{
    void esp_ieee802154_receive_done(uint8_t *frame, esp_ieee802154_frame_info_t *frame_info);
}

static sensor_data_t _sensor_data[128];
static volatile int16_t _sensor_data_head = 0;
static volatile int16_t _sensor_data_tail = 0;

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
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(esp_ieee802154_enable());
    ESP_ERROR_CHECK(esp_ieee802154_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_ieee802154_set_rx_when_idle(true));
    ESP_ERROR_CHECK(esp_ieee802154_set_channel(NET_H2_SENSOR_CHANNEL));
    ESP_ERROR_CHECK(esp_ieee802154_receive());

    for (;;)
    {
        if (_sensor_data_tail != _sensor_data_head)
        {
            while (_sensor_data_tail < _sensor_data_head)
            {
                handleSensorData(&_sensor_data[_sensor_data_tail]);

                _sensor_data_tail += 1;
            }
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}
