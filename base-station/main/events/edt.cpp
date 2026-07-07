#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "events/edt.h"

static QueueHandle_t _edt_job_queue = NULL;

class EventDispatcher
{
public:
    static const size_t MAX_LISTENERS = 8;

    bool add_metric_event_handler(metric_id_t metric_id, metric_event_handler_t cb)
    {
        if (metric_id >= METRIC_COUNT)
        {
            ESP_LOGE("EDT", "Add metric event handler add failed: Invalid id %d [max:%d]", metric_id, METRIC_COUNT);
            return false;
        }

        for (int i = 0; i < MAX_LISTENERS; i++)
        {
            if (!_metric_callbacks[metric_id][i])
            {
                _metric_callbacks[metric_id][i] = cb;

                return true;
            }
        }

        ESP_LOGE("EDT", "Add metric event handler add failed: %d listener count full", metric_id);
        return false;
    }

    bool add_system_event_handler(system_event_id_t event_id, system_event_handler_t cb)
    {
        if (event_id >= SYSTEM_EVENT_COUNT)
        {
            ESP_LOGE("EDT", "Add sytem event handler add failed: Invalid id %d [max:%d]", event_id, SYSTEM_EVENT_COUNT);
            return false;
        }

        for (int i = 0; i < MAX_LISTENERS; i++)
        {
            if (!_event_callbacks[event_id][i])
            {
                _event_callbacks[event_id][i] = cb;

                return true;
            }
        }

        ESP_LOGE("EDT", "Add sytem event handler add failed: %d listener count full", event_id);
        return false;
    }

    void dispatch_metric_event(metric_event_t metric_event)
    {
        if (metric_event.metric_id >= METRIC_COUNT)
        {
            ESP_LOGE("EDT", "Invalid metric event: %d", metric_event.metric_id);
            return;
        }

        for (int i = 0; i < MAX_LISTENERS; i++)
        {
            if (_metric_callbacks[metric_event.metric_id][i])
            {
                _metric_callbacks[metric_event.metric_id][i](metric_event);
            }
        }
    }

    void dispatch_system_event(system_event_t system_event)
    {
        if (system_event.event_id >= SYSTEM_EVENT_COUNT)
        {
            ESP_LOGE("EDT", "Invalid system event: %d", system_event.event_id);
            return;
        }

        for (int i = 0; i < MAX_LISTENERS; i++)
        {
            if (_event_callbacks[system_event.event_id][i])
            {
                _event_callbacks[system_event.event_id][i](system_event);
            }
        }
    }

private:
    metric_event_handler_t _metric_callbacks[METRIC_COUNT][MAX_LISTENERS];
    system_event_handler_t _event_callbacks[SYSTEM_EVENT_COUNT][MAX_LISTENERS];
};

EventDispatcher _event_dispatcher;

extern "C"
{
    void edt_post(edt_job_t job)
    {
        xQueueSend(_edt_job_queue, &job, portMAX_DELAY);
    }

    void edt_add_metric_event_handler(metric_id_t metric_id, metric_event_handler_t callback)
    {
        _event_dispatcher.add_metric_event_handler(metric_id, callback);
    }

    void edt_add_system_event_handler(system_event_id_t event_id, system_event_handler_t callback)
    {
        _event_dispatcher.add_system_event_handler(event_id, callback);
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
                case JOB_TYPE_METRIC_EVENT:
                    _event_dispatcher.dispatch_metric_event(job.metric_event);
                    break;
                case JOB_TYPE_SYSTEM_EVENT:
                    _event_dispatcher.dispatch_system_event(job.system_event);
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

void edt_timer_task(void *pvParameters)
{
    const edt_job_t job = {
        .type = JOB_TYPE_SYSTEM_EVENT,
        .system_event = {
            .event_id = SYSTEM_EVENT_TICK,
            .attributes = 0,
            .value = 0,
        },
    };

    const TickType_t delay_period = pdMS_TO_TICKS(900);

    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        edt_post(job);

        vTaskDelayUntil(&last_wake_time, delay_period);
    }
}

void edt_init()
{
    _edt_job_queue = xQueueCreate(256, sizeof(edt_job_t));

    xTaskCreatePinnedToCore(edt_task, "edt_task", 8192, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(edt_timer_task, "edt_timer_tick_task", 1024, NULL, 2, NULL, 1);
}
