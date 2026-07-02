#include "edt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "edt.h"

static QueueHandle_t _edt_job_queue = NULL;
static Display *_display;

extern "C"
{
    inline Display *active_display() { return _display; }

    void edt_post(edt_job_t job)
    {
        xQueueSend(_edt_job_queue, &job, portMAX_DELAY);
    }

    void process_sensor_data(sensor_data_t *sensor_data)
    {
        char buffer1[128];
        char buffer2[128];
        char buffer3[128];

        printf("P4: %d: %0.2f %0.2f %0.2f (Bat. %d)\n",
               sensor_data->sensor_id,
               sensor_data->reading1,
               sensor_data->reading2,
               sensor_data->reading3,
               sensor_data->battery_mv);

        if (sensor_data->sensor_id == 4)
        {
            sprintf(buffer1, "UV: %0.2f", sensor_data->reading1);

            _display->setValues(0, 0, 0, buffer1);
        }
        else
        {
            sprintf(buffer1, "Temp %0.2fC", sensor_data->reading1);

            sprintf(buffer2, "Humidity: %0.2f%", sensor_data->reading2);

            sprintf(buffer3, "Pressure: %0.2fhPa", sensor_data->reading3);

            _display->setValues(buffer1, buffer2, buffer3, 0);
        }
    }

    void edt_task(void *pvParameters)
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

        xTaskCreatePinnedToCore(edt_task, "edt_ui_task", 8192, NULL, 2, NULL, 1);
    }
}