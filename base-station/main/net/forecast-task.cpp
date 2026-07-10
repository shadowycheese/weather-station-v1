#include <string.h>

#include "net/forecast-task.h"

#include "esp_log.h"
#include "events/edt.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "log/log.h"
#include "events/edt.h"

#include "cJSON.h"
#include "cJSON_Utils.h"

#define HOST "api.open-meteo.com"

#define ONE_HOUR_TICKS pdMS_TO_TICKS(1ULL * 60ULL * 60ULL * 1000ULL)
#define FIVE_MINS_TICKS pdMS_TO_TICKS(5ULL * 60ULL * 1000ULL)
#define READ_BUFFER_SIZE (8192)

static forecast_t _forecast;
static TaskHandle_t _forecast_task_handle = NULL;
static uint8_t *_forecast_json_buffer = NULL;

const char *FC_TAG = "FORECAST";

// https://api.open-meteo.com/v1/forecast?latitude=52.52&longitude=13.41&daily=weather_code,temperature_2m_max,temperature_2m_min,wind_speed_10m_max,wind_gusts_10m_max,wind_direction_10m_dominant,precipitation_probability_max,rain_sum,snowfall_sum,uv_index_max&forecast_days=2&current=temperature_2m,wind_speed_10m,wind_direction_10m,wind_gusts_10m,cloud_cover,weather_code,rain,snowfall

esp_err_t request_forecast()
{
    // Configure target server details and raw HTTP request text
    const char *REQUEST = "GET v1/forecast?latitude=52.52&longitude=13.41&daily=weather_code,temperature_2m_max,temperature_2m_min,wind_speed_10m_max,wind_gusts_10m_max,wind_direction_10m_dominant,precipitation_probability_max,rain_sum,snowfall_sum,uv_index_max&forecast_days=2&current=temperature_2m,wind_speed_10m,wind_direction_10m,wind_gusts_10m,cloud_cover,weather_code,rain,snowfall HTTP/1.1\r\n"
                          "Host: api.open-meteo.com\r\n"
                          "User-Agent: esp32-p4-app\r\n"
                          "Connection: close\r\n"
                          "\r\n";

    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;
    int s;

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

    s = socket(res->ai_family, res->ai_socktype, 0);
    if (s < 0)
    {
        ESP_LOGE(FC_TAG, "Failed to allocate socket.");

        return ESP_ERR_INVALID_STATE;
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
        return ESP_ERR_INVALID_ARG;
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
        return ESP_ERR_NOT_FINISHED;
    }
    else
    {
        ESP_LOGI(FC_TAG, "Socket write complete");
    }

    // 5. Read incoming HTTP payload dynamically into a local buffer
    bzero(_forecast_json_buffer, READ_BUFFER_SIZE);

    for (int offset = 0; offset < READ_BUFFER_SIZE;)
    {
        int r = read(s, _forecast_json_buffer + offset, READ_BUFFER_SIZE - (1 + offset));

        if (r == 0)
        {
            break;
        }
        else if (r < 0)
        {
            close(s);
            return ESP_ERR_INVALID_RESPONSE;
        }

        offset += r;
    };

    // 6. Clean up resources completely
    close(s);

    printf("%s", _forecast_json_buffer);

    return ESP_OK;
}

esp_err_t parse_forecast()
{
    ESP_LOGI(FC_TAG, "Parsing forecast data");
    char *json = strchr((char *)_forecast_json_buffer, '{');

    if (!json)
    {
        ESP_LOGE(FC_TAG, "Could not find json to parse: %s", _forecast_json_buffer);
        return ESP_ERR_INVALID_RESPONSE;
    }

    cJSON *root = cJSON_Parse(json);

    cJSON *daily = cJSON_GetObjectItem(root, "daily");
    cJSON *temps_min = cJSON_GetObjectItem(daily, "temperature_2m_min");
    cJSON *temps_max = cJSON_GetObjectItem(daily, "temperature_2m_max");
    cJSON *wc = cJSON_GetObjectItem(daily, "weather_code");
    cJSON *pop = cJSON_GetObjectItem(daily, "precipitation_probability_max");
    cJSON *wind_speed = cJSON_GetObjectItem(daily, "wind_speed_10m_max");
    cJSON *wind_gusts = cJSON_GetObjectItem(daily, "wind_gusts_10m_max");
    cJSON *wind_dir = cJSON_GetObjectItem(daily, "wind_direction_10m_dominant");
    cJSON *rain = cJSON_GetObjectItem(daily, "rain_sum");
    cJSON *snow = cJSON_GetObjectItem(daily, "snowfall_sum");
    cJSON *uv = cJSON_GetObjectItem(daily, "uv_index_max");
    cJSON *dates = cJSON_GetObjectItem(daily, "time");
    cJSON *current = cJSON_GetObjectItem(root, "current");

    strncpy(_forecast.today.date, cJSON_GetArrayItem(dates, 0)->valuestring, 10);
    _forecast.today.pop = cJSON_GetArrayItem(pop, 0)->valuedouble;
    _forecast.today.rain = cJSON_GetArrayItem(rain, 0)->valuedouble;
    _forecast.today.snow = cJSON_GetArrayItem(snow, 0)->valuedouble;
    _forecast.today.temp_max = cJSON_GetArrayItem(temps_max, 0)->valuedouble;
    _forecast.today.temp_min = cJSON_GetArrayItem(temps_min, 0)->valuedouble;
    _forecast.today.weather_code = cJSON_GetArrayItem(wc, 0)->valueint;
    _forecast.today.wind_speed = cJSON_GetArrayItem(wind_speed, 0)->valuedouble;
    _forecast.today.wind_gust = cJSON_GetArrayItem(wind_speed, 0)->valuedouble;
    _forecast.today.wind_dir = cJSON_GetArrayItem(wind_dir, 0)->valueint;
    _forecast.today.uv_index = cJSON_GetArrayItem(uv, 0)->valuedouble;

    strncpy(_forecast.tomorrow.date, cJSON_GetArrayItem(dates, 1)->valuestring, 10);
    _forecast.tomorrow.pop = cJSON_GetArrayItem(pop, 1)->valuedouble;
    _forecast.tomorrow.rain = cJSON_GetArrayItem(rain, 1)->valuedouble;
    _forecast.tomorrow.snow = cJSON_GetArrayItem(snow, 1)->valuedouble;
    _forecast.tomorrow.temp_max = cJSON_GetArrayItem(temps_max, 1)->valuedouble;
    _forecast.tomorrow.temp_min = cJSON_GetArrayItem(temps_min, 1)->valuedouble;
    _forecast.tomorrow.weather_code = cJSON_GetArrayItem(wc, 1)->valueint;
    _forecast.tomorrow.wind_speed = cJSON_GetArrayItem(wind_speed, 1)->valuedouble;
    _forecast.tomorrow.wind_gust = cJSON_GetArrayItem(wind_speed, 1)->valuedouble;
    _forecast.tomorrow.wind_dir = cJSON_GetArrayItem(wind_speed, 1)->valueint;
    _forecast.tomorrow.uv_index = cJSON_GetArrayItem(uv, 1)->valuedouble;

    strncpy(_forecast.tomorrow.date, "NOW", 10);
    _forecast.current.pop = -1;
    _forecast.current.rain = cJSON_GetObjectItem(current, "rain")->valuedouble;
    _forecast.current.snow = cJSON_GetObjectItem(current, "snowfall")->valuedouble;
    _forecast.current.temp_max = cJSON_GetObjectItem(current, "temperature_2m")->valuedouble;
    _forecast.current.temp_min = _forecast.current.temp_max;
    _forecast.current.weather_code = cJSON_GetObjectItem(current, "weather_code")->valueint;
    _forecast.current.wind_speed = cJSON_GetObjectItem(current, "wind_speed_10m")->valuedouble;
    _forecast.current.wind_gust = cJSON_GetObjectItem(current, "wind_gusts_10m")->valuedouble;
    _forecast.current.wind_dir = cJSON_GetObjectItem(current, "wind_direction_10m")->valuedouble;
    _forecast.current.uv_index = -1;

    cJSON_Delete(root);
    ESP_LOGI(FC_TAG, "Finished parsing forecast JSON: %s: T=%0.1f WC=%d WS=%0.1f POP=%0.1f UV=%0.2f",
             _forecast.today.date,
             _forecast.today.temp_max,
             _forecast.today.weather_code,
             _forecast.today.wind_speed,
             _forecast.today.pop,
             _forecast.today.uv_index);

    return ESP_OK;
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

    TickType_t last_fetch_tick = 0;
    uint32_t notification_value;
    TickType_t next_wake_time = ONE_HOUR_TICKS;

    for (;;)
    {
        BaseType_t from_event = xTaskNotifyWait(0x00, ULONG_MAX, &notification_value, next_wake_time);

        TickType_t current_tick = xTaskGetTickCount();

        if (from_event)
        {
            if ((last_fetch_tick != 0) && ((current_tick - last_fetch_tick) < ONE_HOUR_TICKS))
            {
                ESP_LOGI(FC_TAG, "Wifi connected, but have up to date forecast already");

                next_wake_time = ONE_HOUR_TICKS - (current_tick - last_fetch_tick);

                continue;
            }
        }

        esp_err_t err = request_forecast();

        if (err == ESP_OK)
        {
            err = parse_forecast();

            if (err == ESP_OK)
            {
                last_fetch_tick = current_tick;

                edt_post(job);
            }

            next_wake_time = ONE_HOUR_TICKS;
        }
        else
        {
            ESP_LOGW(FC_TAG, "Failed to get forecast: %d", err);
            next_wake_time = FIVE_MINS_TICKS;
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
        _forecast_json_buffer = (uint8_t *)heap_caps_malloc(READ_BUFFER_SIZE, MALLOC_CAP_SPIRAM);

        app_log(LOG_INFO, FC_TAG, "Creating forecast task");

        edt_add_system_event_handler(SYSTEM_EVENT_WIFI_STATUS,
                                     [](system_event_t m)
                                     {
                                         if (m.value == 1)
                                         {
                                             ESP_LOGI(FC_TAG, "Wifi connected - notifying task");

                                             xTaskNotifyGive(_forecast_task_handle); //
                                         }
                                     });

        xTaskCreateWithCaps(request_forecast_task, "forecast_task", 8192, NULL, 2, &_forecast_task_handle, MALLOC_CAP_INTERNAL);

        return ESP_OK;
    }
}
