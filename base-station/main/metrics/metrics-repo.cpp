#include <sys/stat.h>
#include "esp_littlefs.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "events/edt.h"
#include "metrics/metrics-repo.h"
#include "log/debug.h"
#include "log/log.h"

const char *MR_TAG = "METRICS";

MetricsRepository *MetricsRepository::_instance = NULL;

#define UNBUCKETED_COUNT 10000

#define ONE_HOUR 3600
#define SIX_HOURS (6 * ONE_HOUR)
#define TWELVE_HOURS (12 * ONE_HOUR)
#define ONE_DAY (24 * ONE_HOUR)

constexpr char *_lh_file = "/storage/hl.ts";
constexpr char *_24_hr_file = "/storage/24_hours.ts";
constexpr char *_7_day_file = "/storage/7_days.ts";
constexpr char *_90_day_file = "/storage/90_days.ts";

void init_fs()
{
    esp_vfs_littlefs_conf_t conf =
        {
            .base_path = "/storage",
            .partition_label = "storage",
            .format_if_mount_failed = true,
            .dont_mount = false,
        };

    ESP_ERROR_CHECK(esp_vfs_littlefs_register(&conf));
}

void storage_task(void *)
{
    init_fs();

    MetricsRepository::_instance->load_all();

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(60 * 1000));

        if (!MetricsRepository::_instance->_time_sync_complete)
        {
            continue;
        }

        MetricsRepository::_instance->sync_metrics();
    }
}

void MetricsRepository::init()
{
    _instance = this;

    _latest = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * METRIC_COUNT, MALLOC_CAP_SPIRAM);
    _unbucketed = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * UNBUCKETED_COUNT, MALLOC_CAP_SPIRAM);
    _unbucketed_head = 0;

    _highs_and_lows = static_cast<decltype(_highs_and_lows)>(heap_caps_malloc(_size_highs_and_lows, MALLOC_CAP_SPIRAM));
    _24_hours = static_cast<decltype(_24_hours)>(heap_caps_malloc(_size_24_hours, MALLOC_CAP_SPIRAM));
    _7_days = static_cast<decltype(_7_days)>(heap_caps_malloc(_size_7_days, MALLOC_CAP_SPIRAM));
    _90_days = static_cast<decltype(_90_days)>(heap_caps_malloc(_size_90_days, MALLOC_CAP_SPIRAM));

    for (int i = 0; i < METRIC_COUNT; i++)
    {
        _latest[i].timestamp = -1LL;
        _latest[i].sensor_id = _metric_map[i];
        _latest[i].metric_id = (metric_id_t)i;
    }

    for (int i = 0; i < UNBUCKETED_COUNT; i++)
    {
        _unbucketed[i].timestamp = -1LL;
    }

    for (int m = 0; m < METRIC_COUNT; m++)
    {
        for (int i = 0; i < HIGH_LOW_COUNT; i++)
        {
            _highs_and_lows[m][i].timestamp = -1LL;
        }
        for (int i = 0; i < ONE_DAY_INTERVAL_COUNT; i++)
        {
            _24_hours[m][i].timestamp = -1LL;
        }
        for (int i = 0; i < SEVEN_DAY_INTERVAL_COUNT; i++)
        {
            _7_days[m][i].timestamp = -1LL;
        }
        for (int i = 0; i < NINETY_DAY_INTERVAL_COUNT; i++)
        {
            _90_days[m][i].timestamp = -1LL;
        }
    }

    app_log(LOG_INFO, MR_TAG, "Element size = %d", sizeof(timeseries_data_t));
    app_log(LOG_INFO, MR_TAG, "Array row sizes: 24h=%d 7d=%d 90d=%d HL=%d", sizeof(*_24_hours), sizeof(*_7_days), sizeof(*_90_days), sizeof(*_highs_and_lows));
    app_log(LOG_INFO, MR_TAG, "Allocated = %d", (UNBUCKETED_COUNT * sizeof(timeseries_data_t)) + _size_highs_and_lows + _size_24_hours + _size_7_days + _size_90_days);
    app_log(LOG_INFO, MR_TAG, "Storage Required = %d (24h=%d, 7d=%d, 90d=%d, HL=%d)",
            _size_24_hours + _size_7_days + _size_90_days + _size_highs_and_lows,
            _size_24_hours,
            _size_7_days,
            _size_90_days,
            _size_highs_and_lows);

    edt_add_system_event_handler(SYSTEM_EVENT_TIME_UPDATED, [this](system_event_t ev)
                                 {
                                     time_t t = time(NULL);

                                     struct tm timeinfo;

                                     localtime_r(&t, &timeinfo);

                                     _last_hour = timeinfo.tm_hour;
                                     _last_5_min = timeinfo.tm_min / 5;
                                     _last_day = timeinfo.tm_wday;

                                     _time_sync_complete = true; //
                                 });

    xTaskCreateWithCaps(storage_task, "storage_task", 8192, NULL, 1, &_storage_task_handle, MALLOC_CAP_SPIRAM);
}

void MetricsRepository::sync_5_min_inteval(time_t now)
{
    timeseries_data_t *in_next_bucket = _unbucketed;
    timeseries_data_t *processing = _unbucketed;

    float min[METRIC_COUNT]{};
    float max[METRIC_COUNT]{};
    float values[METRIC_COUNT]{};
    int value_count[METRIC_COUNT]{};

    bool min_set[METRIC_COUNT]{};
    bool max_set[METRIC_COUNT]{};

    time_t interval_start = now - (now % 300);
    time_t interval_end = interval_start + 300;

    for (int i = 0; i < _unbucketed_head; i++)
    {
        timeseries_data_t *curr = processing++;

        time_t event_time = curr->timestamp;

        if (event_time >= interval_end)
        {
            *in_next_bucket++ = *curr;
        }
        else
        {
            sensor_id_t sensor_id = curr->sensor_id;
            value_count[sensor_id]++;
            values[sensor_id] += curr->average;

            if ((min_set[sensor_id] == false) || (curr->min < min[sensor_id]))
            {
                min[sensor_id] = curr->min;
                min_set[sensor_id] = true;
            }
            if ((max_set[sensor_id] == false) || (curr->max > max[sensor_id]))
            {
                max[sensor_id] = curr->max;
                max_set[sensor_id] = true;
            }
        }
    }

    _unbucketed_head = in_next_bucket - _unbucketed;

    for (uint16_t i = (metric_id_t)0; i < METRIC_COUNT; i++)
    {
        timeseries_data_t &ts = _24_hours[_24_hour_head][i];

        ts = {};
        ts.timestamp = now;
        ts.sensor_id = _metric_map[i];
        ts.metric_id = (metric_id_t)i;

        if (value_count[i] == 0)
        {
            ts.time_series_attributes = TS_IS_VALID | TS_NO_VALUE,
            ts.max = 0.0f;
            ts.min = 0.0f;
            ts.average = 0.0f;
        }
        else
        {
            ts.time_series_attributes = TS_IS_VALID;
            ts.max = max[i];
            ts.min = min[i];
            ts.average = values[i] / value_count[i];
        }
    }

    FILE *f = fopen(_24_hr_file, "r+b");

    if (f == NULL)
    {
        ESP_LOGE(MR_TAG, "Could not save metrics at row %d in file %s", _24_hour_head, _24_hr_file);

        return;
    }

    long seek_pos = sizeof(*_24_hours) * _24_hour_head;

    int err = fseek(f, seek_pos, SEEK_SET);

    if (err != 0)
    {
        ESP_LOGE(MR_TAG, "Could not save metrics at row %d in file %s [seek to %ld]", _24_hour_head, _24_hr_file, seek_pos);

        fclose(f);

        return;
    }

    void *write_from = &_24_hours[_24_hour_head];

    size_t written = fwrite(write_from, 1, sizeof(*_24_hours), f);

    fflush(f);
    fclose(f);

    if (written != sizeof(*_24_hours))
    {
        ESP_LOGE(MR_TAG, "Failed to write enough bytes at row %d in file %s [seek to %ld], wrote %ld, expected %ld", _24_hour_head, _24_hr_file, seek_pos, written, sizeof(*_24_hours));

        return;
    }

    _24_hour_head = (_24_hour_head + 1) % ONE_DAY_INTERVAL_COUNT;
}

void MetricsRepository::sync_1_hour_interval(time_t now) {}
void MetricsRepository::sync_1_day_interval(time_t now) {}

void MetricsRepository::sync_metrics()
{
    time_t t = time(NULL);

    struct tm timeinfo;

    localtime_r(&t, &timeinfo);

    int five_min = timeinfo.tm_min / 5;
    int hour = timeinfo.tm_hour;
    int day = timeinfo.tm_wday;

    if (five_min != _last_5_min)
    {
    }
}

void MetricsRepository::load_all()
{
    load_metrics_file(_lh_file, (void *)_highs_and_lows, _size_highs_and_lows);
    load_metrics_file(_24_hr_file, (void *)_24_hours, _size_24_hours);
    load_metrics_file(_7_day_file, (void *)_7_days, _size_7_days);
    load_metrics_file(_90_day_file, (void *)_90_days, _size_90_days);

    _24_hour_head = 0;
    _7_day_head = 0;
    _90_day_head = 0;

    for (int i = 0; i < ONE_DAY_INTERVAL_COUNT; i++)
    {
        if (_24_hours[0][i].timestamp = -1LL)
        {
            _24_hour_head = i;
            break;
        }
    }
    for (int i = 0; i < SEVEN_DAY_INTERVAL_COUNT; i++)
    {
        if (_7_days[0][i].timestamp = -1LL)
        {
            _7_day_head = i;
            break;
        }
    }
    for (int i = 0; i < NINETY_DAY_INTERVAL_COUNT; i++)
    {
        if (_90_days[0][i].timestamp = -1LL)
        {
            _90_day_head = i;
            break;
        }
    }
}

void MetricsRepository::load_metrics_file(const char *file_name, void *destination, size_t size)
{
    struct stat st;

    if (stat(file_name, &st) == 0)
    {
        if (st.st_size != size)
        {
            ESP_LOGE(MR_TAG, "File %s has an unexpected (expected %ld, was %ld)", file_name, size, st.st_size);

            reset_file(file_name, destination, size);

            return;
        }
    }
    else
    {
        ESP_LOGE(MR_TAG, "%s does not exist... writing defaults", file_name);

        reset_file(file_name, destination, size);

        return;
    }

    FILE *f = fopen(file_name, "rb");

    if (f == NULL)
    {
        ESP_LOGE(MR_TAG, "%s could not open file", file_name);

        reset_file(file_name, destination, size);

        return;
    }

    if (fread(destination, 1, size, f) != size)
    {
        fclose(f);

        ESP_LOGE(MR_TAG, "%s: not all bytes read from file", file_name);

        reset_file(file_name, destination, size);

        return;
    }

    fclose(f);

    ESP_LOGI(MR_TAG, "%s loaded", file_name);
}

void MetricsRepository::reset_file(const char *file_name, void *destination, size_t size)
{
    FILE *f = fopen(file_name, "wb");

    if (f == NULL)
    {
        ESP_LOGE(MR_TAG, "Could not create file:%s", file_name);

        return;
    }

    if (fwrite(destination, 1, size, f) != size)
    {
        ESP_LOGE(MR_TAG, "Could write %ld bytes to file %s", size, file_name);
    }
    else
    {
        ESP_LOGW(MR_TAG, "File %s created", file_name);
    }

    fclose(f);

    return;
}

void MetricsRepository::handle_sensor_data(sensor_data_t sensor_data)
{
    _instance->update(&sensor_data);
}

void MetricsRepository::update(sensor_data_t *sd)
{
    debug_sensor("REPO", sd);

    (this->*_sensor_updates[sd->sensor_id])(sd);
}

void MetricsRepository::update_inside_bme280(sensor_data_t *sd)
{
    update_time_series_data(sd, METRIC_INSIDE_BME280_TEMPERATURE, sd->reading1);
    update_time_series_data(sd, METRIC_INSIDE_BME280_HUMIDITY, sd->reading2);
    update_time_series_data(sd, METRIC_INSIDE_BME280_PRESSURE, sd->reading3);
}

void MetricsRepository::update_inside_sgp30(sensor_data_t *sd)
{
    update_time_series_data(sd, METRIC_INSIDE_SGP30_ECO2, sd->reading1);
    update_time_series_data(sd, METRIC_INSIDE_SGP30_TVOC, sd->reading2);
    update_time_series_data(sd, METRIC_INSIDE_SGP30_AQI, calculate_inside_aqi());
}

void MetricsRepository::update_outside1_bme280(sensor_data_t *sd)
{
    update_time_series_data(sd, METRIC_OUTSIDE1_BME280_TEMPERATURE, sd->reading1);
    update_time_series_data(sd, METRIC_OUTSIDE1_BME280_HUMIDITY, sd->reading2);
    update_time_series_data(sd, METRIC_OUTSIDE1_BME280_PRESSURE, sd->reading3);
    update_time_series_data(sd, METRIC_OUTSIDE1_BME280_BATTERY, sd->battery_mv);
}

void MetricsRepository::update_outside_uv(sensor_data_t *sd)
{
    update_time_series_data(sd, METRIC_OUTSIDE_UV_RAW, sd->reading1);
    update_time_series_data(sd, METRIC_OUTSIDE_UV_INDEX, calculate_uvi());
    update_time_series_data(sd, METRIC_OUTSIDE_UV_BATTERY, sd->battery_mv);
}

void MetricsRepository::update_outside_sps30_set1(sensor_data_t *sd)
{
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM1P0MASS, sd->reading1);
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM2P5MASS, sd->reading2);
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM4P0MASS, sd->reading3);
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_BATTERY, sd->battery_mv);
}

void MetricsRepository::update_outside_sps30_set2(sensor_data_t *sd)
{
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM10MASS, sd->reading1);
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM0P5NUMBER, sd->reading2);
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM1P0NUMBER, sd->reading3);
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_BATTERY, sd->battery_mv);
}

void MetricsRepository::update_outside_sps30_set3(sensor_data_t *sd)
{
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM2P5NUMBER, sd->reading1);
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM4P0NUMBER, sd->reading2);
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM10NUMBER, sd->reading3);
    update_time_series_data(sd, METRIC_OUTSIDE_SPS30_BATTERY, sd->battery_mv);
}

void MetricsRepository::update_outside_rain_gauge(sensor_data_t *sd)
{
    update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_TICKS, sd->reading1);
    update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_MM_HOUR, calculate_rainfall(ONE_HOUR));
    update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_MM_6HOURS, calculate_rainfall(SIX_HOURS));
    update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_MM_12HOURS, calculate_rainfall(TWELVE_HOURS));
    update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_MM_24HOURS, calculate_rainfall(ONE_DAY));
    update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_BATTERY, sd->battery_mv);
}

float MetricsRepository::calculate_uvi()
{
    return 0;
}

float MetricsRepository::calculate_inside_aqi()
{
    return 0;
}

float MetricsRepository::calculate_rainfall(int in_last_seconds)
{
    return 0;
}

void MetricsRepository::update_time_series_data(sensor_data_t *sensor_data, metric_id_t id, float value)
{
    uint16_t attributes = 0;

    _latest[id].timestamp = time(NULL);
    _latest[id].average = value;
    _latest[id].min = value;
    _latest[id].max = value;

    if (_highs_and_lows[id][HIGH_24H].timestamp < 0 || _highs_and_lows[id][HIGH_24H].max < value)
    {
        _highs_and_lows[id][HIGH_24H].average = value;
        _highs_and_lows[id][HIGH_24H].min = value;
        _highs_and_lows[id][HIGH_24H].max = value;
        _highs_and_lows[id][HIGH_24H].timestamp = _latest[id].timestamp;

        attributes |= IS_24_HR_HIGH;
    }
    if (_highs_and_lows[id][LOW_24H].timestamp < 0 || _highs_and_lows[id][LOW_24H].min > value)
    {
        _highs_and_lows[id][LOW_24H].average = value;
        _highs_and_lows[id][LOW_24H].min = value;
        _highs_and_lows[id][LOW_24H].max = value;
        _highs_and_lows[id][LOW_24H].timestamp = _latest[id].timestamp;

        attributes |= IS_24_HR_LOW;
    }

    metric_event_t event = {
        .metric_id = id,
        .attributes = attributes,
        .value = value,
    };

    edt_job_t job = {
        .type = JOB_TYPE_METRIC_EVENT,
        .metric_event = event,
    };

    edt_post(job);
}
