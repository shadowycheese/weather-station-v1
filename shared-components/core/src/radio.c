#include "models.h"
#include "radio.h"
#include "esp_ieee802154.h" // Native 15.4 low-level radio controller
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const uint8_t NET_P4_MAC[6] = {0x80, 0xF1, 0xB2, 0xD2, 0xBC, 0x93};
const uint8_t NET_H2_RECEIVER_MAC[6] = {0x74, 0x4D, 0xBD, 0x68, 0x63, 0xA8};
const uint8_t NET_H2_SENSOR_TEMP_1_MAC[6] = {0x74, 0x4D, 0xBD, 0x68, 0x5C, 0x33};

int ieee_802154_transmit_sensor_data(sensor_data_t *sensorData)
{
    ESP_ERROR_CHECK(esp_ieee802154_enable());
    ESP_ERROR_CHECK(esp_ieee802154_set_channel(NET_H2_SENSOR_CHANNEL));

    uint8_t frame[128];

    int idx = 1;

    frame[idx++] = 0x01; // Frame Type: Data Frame (Bits 0-2)
    frame[idx++] = 0x08; // Addressing: No destination address, Source address is short (16-bit)

    static uint8_t seq = 0;
    frame[idx++] = seq++; // Sequence Number

    frame[idx++] = NET_H2_SENSOR_PAN_ID & 0xFF;
    frame[idx++] = (NET_H2_SENSOR_PAN_ID >> 8) & 0xFF;
    frame[idx++] = NET_H2_SENSOR_TEMP_1 & 0xFF;
    frame[idx++] = (NET_H2_SENSOR_TEMP_1 >> 8) & 0xFF;

    // Copy your raw sensor variables into the frame payload area
    memcpy(&frame[idx], sensorData, sizeof(sensor_data_t));
    idx += sizeof(sensor_data_t);

    uint8_t total_length = idx + 2;

    int result = esp_ieee802154_transmit(frame, false);

    frame[0] = total_length;

    vTaskDelay(pdMS_TO_TICKS(10));

    if (result == ESP_OK)
    {
        printf("TX: %d: %0.2f %0.2f %0.2f (Bat. %d)\n",
               sensorData->node_id,
               sensorData->reading1,
               sensorData->reading2,
               sensorData->reading3,
               sensorData->battery_mv);
    }
    else
    {
        printf("TX: FAILED\n");
    }

    // 5. Clean up radio and enter low-power deep sleep
    esp_ieee802154_disable();

    return result;
}