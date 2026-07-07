#include "net/forecast.h"
/*
#include "esp_http_client.h"
#include "esp_log.h"
#include "events/edt.h"

#define URL "http://api.open-meteo.com/v1/forecast?latitude=49.232&longitude=-122.459&daily=weather_code,temperature_2m_max,precipitation_probability_max,wind_speed_10m_max,shortwave_radiation_sum&timezone=auto&forecast_days=2"

static forecast_t _forecast;

const char *FC_TAG = "FORECAST";

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    printf("%d\n", evt->data_len);

    return ESP_OK;
}

esp_err_t request_forecast()
{
    esp_http_client_config_t config = {
        .url = URL,
        .method = HTTP_METHOD_GET,
        .event_handler = http_event_handler,
        .buffer_size = 1024,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    return err;
}

void request_forecast_task(void *pvParameters)
{
    const edt_job_t job = {
        .type = JOB_TYPE_SYSTEM_EVENT,
        .system_event = {
            .event_id = SYSTEM_EVENT_FORECAST,
            .attributes = 0,
            .value = 0,
        },
    };

    const TickType_t delay_period = pdMS_TO_TICKS(1000 * 60 * 60); // Hourly
    const TickType_t delay_period_err = pdMS_TO_TICKS(1000 * 5);   // Every 5s

    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        esp_err_t err = request_forecast();

        if (err == ESP_OK)
        {
            edt_post(job);
            vTaskDelayUntil(&last_wake_time, delay_period);
        }
        else
        {
            ESP_LOGW(FC_TAG, "Failed to get forecast: %d", err);
            vTaskDelayUntil(&last_wake_time, delay_period_err);
        }
    }
}

extern "C"
{
    esp_err_t get_forecast(forecast_t *fc)
    {
        memcpy(fc, &_forecast, sizeof(forecast_t));

        return ESP_OK;
    }

    esp_err_t forecast_init()
    {
        xTaskCreatePinnedToCore(request_forecast_task, "edt_timer_tick_task", 1024, NULL, 2, NULL, 1);

        return ESP_OK;
    }
}
*/