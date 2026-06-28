#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_ieee802154.h"
#include "esp_sleep.h"
#include "models.h"
#include "nvs_flash.h"

extern "C" void app_main(void)
{
    nvs_flash_init();

    sensor_data_t sensorData;

    sensorData.battery_mv = 100;
    sensorData.node_id = NET_H2_SENSOR_TEMP_1;
    sensorData.reading1 = 0.5f;
    sensorData.reading2 = 32.5f;
    sensorData.reading3 = 88.5f;

    for (;;)
    {
        ieee_802154_transmit_sensor_data(&sensorData);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
