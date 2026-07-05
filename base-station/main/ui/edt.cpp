#include "edt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "edt.h"

static QueueHandle_t _edt_job_queue = NULL;

class MetricEventDispatcher
{
public:
    static const size_t MAX_LISTENERS = 8;

    // Add a new listener slot safely
    bool add_listener(metric_id_t metric_id, metric_event_handler_t cb)
    {
        if (metric_id >= METRIC_COUNT)
        {
            ESP_LOGE("METRIC_EVENT", "Registration failed: Invalid metric id %d", metric_id);
            return false;
        }

        bool added = false;

        for (int i = 0; i < MAX_LISTENERS; i++)
        {
            if (!_callbacks[metric_id][i])
            {
                _callbacks[metric_id][i] = cb;
                added = true;
                break;
            }
        }

        if (added == false)
        {
            ESP_LOGE("METRIC_EVENT", "Registration failed: Metric %d has reached maximum listener capacity!", metric_id);
            return false;
        }

        return true;
    }

    // Zero-logic execution dispatch loop
    void dispatch(metric_event_t metric_event)
    {
        // Safe check for bounds
        if (metric_event.metric_id >= METRIC_COUNT)
        {
            return;
        }

        // Loop through only the slots that have been actively filled
        for (size_t i = 0; i < MAX_LISTENERS; i++)
        {
            if (!_callbacks[metric_event.metric_id][i])
            {
                break;
            }

            _callbacks[metric_event.metric_id][i](metric_event);
        }
    }

private:
    metric_event_handler_t _callbacks[METRIC_COUNT][MAX_LISTENERS];
};

MetricEventDispatcher _eventDispatcher;

extern "C"
{
    void edt_post(edt_job_t job)
    {
        xQueueSend(_edt_job_queue, &job, portMAX_DELAY);
    }

    void edt_register(metric_id_t metric_id, metric_event_handler_t callback)
    {
        _eventDispatcher.add_listener(metric_id, callback);
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
                case JOB_TYPE_METRIC:
                    _eventDispatcher.dispatch(job.metric_event);
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

void edt_init()
{
    _edt_job_queue = xQueueCreate(256, sizeof(edt_job_t));

    xTaskCreatePinnedToCore(edt_task, "edt_ui_task", 8192, NULL, 2, NULL, 1);
}
