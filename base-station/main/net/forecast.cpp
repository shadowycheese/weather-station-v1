#include "net/forecast.h"

#include "esp_log.h"
#include "events/edt.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "log/log.h"

#define HOST "api.open-meteo.com"
#define GET "/v1/forecast?latitude=49.232&longitude=-122.459&daily=weather_code,temperature_2m_max,precipitation_probability_max,wind_speed_10m_max,shortwave_radiation_sum&timezone=auto&forecast_days=2"

static forecast_t _forecast;

const char *FC_TAG = "FORECAST";

esp_err_t request_forecast()
{
    // Configure target server details and raw HTTP request text
    const char *REQUEST = "GET /v1/forecast?latitude=49.232&longitude=-122.459&daily=weather_code,temperature_2m_max,precipitation_probability_max,wind_speed_10m_max,shortwave_radiation_sum&timezone=auto&forecast_days=2 HTTP/1.1\r\n"
                          "Host: api.open-meteo.com\r\n"
                          "User-Agent: esp32-p4-app\r\n"
                          "Connection: close\r\n"
                          "\r\n";
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    int s;

    // 1. Resolve host name via DNS (uses existing lwip stack seamlessly)
    int err = getaddrinfo(HOST, "80", &hints, &res);
    if (err != 0 || res == NULL)
    {
        ESP_LOGE(FC_TAG, "DNS lookup failed err=%d", err);
        return err;
    }
    else
    {
        ESP_LOGI(FC_TAG, "Found address %s", res->ai_addr->sa_data);
    }

    // 2. Allocate the socket dynamically from the wide-open main heap
    s = socket(res->ai_family, res->ai_socktype, 0);
    if (s < 0)
    {
        ESP_LOGE(FC_TAG, "Failed to allocate socket.");
        return ESP_ERR_INVALID_STATE;
        ;
    }
    else
    {
        ESP_LOGI(FC_TAG, "Socket created");
    }

    // 3. Connect to the server
    if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
    {
        ESP_LOGE(FC_TAG, "Socket connection failed");
        close(s);
        return ESP_ERR_INVALID_STATE;
    }
    else
    {
        ESP_LOGI(FC_TAG, "Socket connected");
    }
    freeaddrinfo(res); // Free addrinfo structure immediately to save RAM

    // 4. Transmit the raw HTTP stream
    if (write(s, REQUEST, strlen(REQUEST)) < 0)
    {
        ESP_LOGE(FC_TAG, "Socket write failed");
        close(s);
        return ESP_ERR_INVALID_STATE;
    }
    else
    {
        ESP_LOGI(FC_TAG, "Socket write complete");
    }

    // 5. Read incoming HTTP payload dynamically into a local buffer
    char recv_buf[128];
    int r;
    do
    {
        bzero(recv_buf, sizeof(recv_buf));
        r = read(s, recv_buf, sizeof(recv_buf) - 1);
        if (r > 0)
        {
            // Print chunk data out or safely pass it to your display UI string buffers
            printf("%s", recv_buf);
        }
    } while (r > 0);

    // 6. Clean up resources completely
    close(s);

    return ESP_OK;
}

void noop_task(void *pvParameters)
{
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
        app_log(LOG_INFO, FC_TAG, "Creating forecast task");

        xTaskCreateWithCaps(request_forecast_task, "forecast_task", 8192, NULL, 2, NULL, MALLOC_CAP_INTERNAL);

        return ESP_OK;
    }
}
