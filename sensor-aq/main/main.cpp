#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_ieee802154.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "models.h"
#include "espuart.h"
#include "radio.h"
#include "espio.h"
#include "sps30/sps30.h"
#include "math.h"
#include "driver/uart.h"

#define UART_PORT UART_NUM_1
#define UART_RX_PIN GPIO_NUM_4
#define UART_TX_PIN GPIO_NUM_5
#define UART_BUF_SIZE 256
#define UART_SPEED 460800

#define TICK_INTERVAL 1000
#define SPS30_TICKS_BEFORE_READ 15
#define SPS30_TICKS_PER_SEND 10

static const char *TAG = "AQ";

uint32_t _task_ticks = 0;
#define DEEP_SLEEP_MILLIS (10ULL * 1000ULL * 1000ULL)
#define ERROR_DEEP_SLEEP_MILLIS (5ULL * 1000ULL * 1000ULL)

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

    //  enter_deep_sleep(ERROR_DEEP_SLEEP_MILLIS);
}

void handle_error(const char *msg)
{
    ESP_LOGE(TAG, "%s", msg);

    //  enter_deep_sleep(ERROR_DEEP_SLEEP_MILLIS);
}

extern "C" void app_main(void)
{
    ensure_success(nvs_flash_init());

    start_uart(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, 256, 115200);

    ensure_success(sps30_wake_up());

    struct sps30_measurement measurement;

    for (;;)
    {
        ensure_success(sps30_start_measurement());

        ensure_success(sps30_read_measurement(&measurement));

        sensor_data_t data = {
            .sensor_id = SENSOR_OUTSIDE_SPS30_SET1,
            .reading1 = measurement.mc_1p0,
            .reading2 = measurement.mc_2p5,
            .reading3 = measurement.mc_4p0,
            .battery_mv = -1,
        };

        ieee_802154_transmit_sensor_data(SENSOR_OUTSIDE_SPS30_SET1, &data);

        data.reading1 = measurement.mc_10p0;
        data.reading2 = measurement.nc_0p5;
        data.reading3 = measurement.nc_1p0;
        data.sensor_id = SENSOR_OUTSIDE_SPS30_SET2;

        ieee_802154_transmit_sensor_data(SENSOR_OUTSIDE_SPS30_SET2, &data);

        data.reading1 = measurement.nc_2p5;
        data.reading2 = measurement.nc_4p0;
        data.reading3 = measurement.nc_10p0;
        data.sensor_id = SENSOR_OUTSIDE_SPS30_SET3;

        ieee_802154_transmit_sensor_data(SENSOR_OUTSIDE_SPS30_SET3, &data);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    ensure_success(sps30_sleep());

    enter_deep_sleep(DEEP_SLEEP_MILLIS);
}