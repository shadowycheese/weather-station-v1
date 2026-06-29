#include "edt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"

static QueueHandle_t _edt_job_queue = NULL;
static Display *_display;

extern "C"
{
    inline Display *active_display() { return _display; }

    void edt_post(edt_job_t job)
    {
        xQueueSend(_edt_job_queue, &job, portMAX_DELAY);
    }

    void process_sensor_data(sensor_data_t *sensorData)
    {
        char buffer[128];

        printf("P4: %d: %0.2f %0.2f %0.2f (Bat. %d)\n",
               sensorData->node_id,
               sensorData->reading1,
               sensorData->reading2,
               sensorData->reading3,
               sensorData->battery_mv);

        sprintf(buffer, "%0.2f", sensorData->reading1);

        _display->setText(buffer);
    }

    void edit_task(void *pvParameters)
    {
        edt_job_t job;

        for (;;)
        {
            // Block indefinitely until ANY producer posts a job to the queue
            if (xQueueReceive(_edt_job_queue, &job, portMAX_DELAY) == pdTRUE)
            {
                if (bsp_display_lock(portMAX_DELAY) == ESP_OK)
                {
                    switch (job.type)
                    {
                    case JOB_TYPE_SENSOR_DATA:
                        process_sensor_data(&job.payload.sensor_data);
                        break;
                    case JOB_TYPE_LAMBDA:
                        job.execute();
                        break;
                    default:
                        break;
                    }
                    lv_timer_handler();
                    bsp_display_unlock();
                }
            }
        }
    }

    void edt_init(Display *display)
    {
        _display = display;

        _edt_job_queue = xQueueCreate(256, sizeof(edt_job_t));

        xTaskCreatePinnedToCore(edit_task, "edt_ui_task", 8192, NULL, 2, NULL, 1);
    }
}