#include "data/remote-sensor.h"
#include "ui/edt.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "espio.h"
#include "espuart.h"
#include "data/db.h"

static const char *TAG = "REMOTE-SENSOR";

#define H2_UART_PORT UART_NUM_1
#define H2_TX_PIN GPIO_NUM_20
#define H2_RX_PIN GPIO_NUM_21
#define H2_HANDSHAKE_PIN GPIO_NUM_22
#define BUF_SIZE 256
#define UART_SPEED 460800

typedef enum
{
    REMOTE_SENSOR_DATA,
} sensor_type_t;

typedef struct
{
    sensor_type_t type;
} sensor_job_t;

static QueueHandle_t _sensor_job_queue = NULL;
static TaskHandle_t _sensor_task_handle;

void sensor_uart_isr(void *_)
{
    if (gpio_get_level(H2_HANDSHAKE_PIN) != 1)
    {
        return;
    }

    sensor_job_t job = {};

    job.type = REMOTE_SENSOR_DATA;

    BaseType_t task_awake;

    xQueueSendToBackFromISR(_sensor_job_queue, &job, &task_awake);

    if (task_awake == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

void uart_read()
{
    edt_job_t job = {};

    job.type = JOB_TYPE_SENSOR_DATA;
    job.payload.sensor_data = {};

    static uint8_t uart_rx_buffer[BUF_SIZE];

    vTaskDelay(pdMS_TO_TICKS(2));

    int bytes_read = uart_read_bytes(H2_UART_PORT, uart_rx_buffer, BUF_SIZE - 1, pdMS_TO_TICKS(10));

    if (bytes_read > 0)
    {
        uart_rx_buffer[bytes_read] = '\0';
    }

    memcpy(&job.payload.sensor_data, uart_rx_buffer, sizeof(sensor_data_t));

    Db::handle_sensor_data(job.payload.sensor_data);

    edt_post(job);
}

void uart_task(void *pvParameters)
{
    sensor_job_t job;

    for (;;)
    {
        if (xQueueReceive(_sensor_job_queue, &job, portMAX_DELAY) == pdTRUE)
        {
            switch (job.type)
            {
            case REMOTE_SENSOR_DATA:
                uart_read();
                break;
            }
        }
    }
}

extern "C"
{
    void remote_sensor_read_start()
    {
        // ---- A. Configure UART Peripherals ----
        start_uart(H2_UART_PORT, H2_TX_PIN, H2_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, BUF_SIZE * 2, UART_SPEED);

        _sensor_job_queue = xQueueCreate(256, sizeof(sensor_job_t));

        app_log(LOG_INFO, TAG, "Starting remote sensor receiver task");

        xTaskCreatePinnedToCore(uart_task, "remote_sensors", 4096, NULL, 3, &_sensor_task_handle, 0);

        configure_input_pin(H2_HANDSHAKE_PIN, GPIO_INTR_POSEDGE);

        gpio_isr_handler_add(H2_HANDSHAKE_PIN, sensor_uart_isr, (void *)H2_HANDSHAKE_PIN);
    }
}