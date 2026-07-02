#include "data/remote-sensor.h"
#include "ui/edt.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "espio.h"
#include "data/db.h"

static const char *TAG = "REMOTE-SENSOR";

static spi_device_handle_t _spi_handle = NULL;

#define GPIO_MOSI 1
#define GPIO_MISO 2
#define GPIO_SCLK 3
#define GPIO_CS 4
#define GPIO_HANDSHAKE GPIO_NUM_5

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

DMA_ATTR static uint8_t _spi_rx_buffer[128];

void sensor_spi_isr(void *_)
{
    if (gpio_get_level(GPIO_HANDSHAKE) != 1)
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

void spi_read()
{
    edt_job_t job = {};

    job.type = JOB_TYPE_SENSOR_DATA;
    job.payload.sensor_data = {};

    spi_transaction_t t = {};

    t.length = sizeof(sensor_data_t) * 8;
    t.tx_buffer = NULL;
    t.rx_buffer = _spi_rx_buffer;

    if (spi_device_polling_transmit(_spi_handle, &t) == ESP_OK)
    {
        memcpy(&job.payload.sensor_data, _spi_rx_buffer, sizeof(sensor_data_t));

        Db::handle_sensor_data(job.payload.sensor_data);

        edt_post(job);
    }
}

void spi_task(void *pvParameters)
{
    sensor_job_t job;

    for (;;)
    {
        // Block indefinitely until ANY producer posts a job to the queue
        if (xQueueReceive(_sensor_job_queue, &job, portMAX_DELAY) == pdTRUE)
        {
            switch (job.type)
            {
            case REMOTE_SENSOR_DATA:
                spi_read();
                break;
            }
        }
    }
}

extern "C"
{
    void remote_sensor_read_start()
    {
        _spi_handle = init_spi_master(SPI2_HOST,
                                      GPIO_MOSI,
                                      GPIO_MISO,
                                      GPIO_SCLK,
                                      GPIO_CS,
                                      GPIO_HANDSHAKE);

        _sensor_job_queue = xQueueCreate(256, sizeof(sensor_job_t));

        app_log(LOG_INFO, TAG, "Starting remote sensor receiver task");

        xTaskCreatePinnedToCore(spi_task, "remote_sensors", 4096, NULL, 3, &_sensor_task_handle, 0);

        configure_input_pin(GPIO_HANDSHAKE, GPIO_INTR_POSEDGE);

        gpio_isr_handler_add(GPIO_HANDSHAKE, sensor_spi_isr, (void *)GPIO_HANDSHAKE);
    }
}