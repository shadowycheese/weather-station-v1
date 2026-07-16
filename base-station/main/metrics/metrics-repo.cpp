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

constexpr char const *_lh_file = "/storage/hl.ts";
constexpr char const *_24_hr_file = "/storage/24_hours.ts";
constexpr char const *_7_day_file = "/storage/7_days.ts";
constexpr char const *_90_day_file = "/storage/90_days.ts";

void init_fs()
{
    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = "/storage";
    conf.partition_label = "storage";
    conf.format_if_mount_failed = true;
    conf.dont_mount = false;

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

    _highs_and_lows_24_hours = (timeseries_data_t *)heap_caps_malloc(_size_highs_and_lows_24_hours, MALLOC_CAP_SPIRAM);
    _highs_and_lows_all_time = (timeseries_data_t *)heap_caps_malloc(_size_highs_and_lows_all_time, MALLOC_CAP_SPIRAM);
    _24_hours = (timeseries_data_t *)heap_caps_malloc(_size_24_hours, MALLOC_CAP_SPIRAM);
    _7_days = (timeseries_data_t *)heap_caps_malloc(_size_7_days, MALLOC_CAP_SPIRAM);
    _90_days = (timeseries_data_t *)heap_caps_malloc(_size_90_days, MALLOC_CAP_SPIRAM);

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

    for (metric_id_t m = (metric_id_t)0; m < METRIC_COUNT; m = (metric_id_t)(m + 1))
    {
        for (int i = 0; i < HIGH_LOW_COUNT; i++)
        {
            _highs_and_lows_24_hours[ts_index(m, i, HIGH_LOW_COUNT)].timestamp = -1LL;
            _highs_and_lows_all_time[ts_index(m, i, HIGH_LOW_COUNT)].timestamp = -1LL;
        }
        for (int i = 0; i < ONE_DAY_INTERVAL_COUNT; i++)
        {
            _24_hours[ts_index(m, i, ONE_DAY_INTERVAL_COUNT)].timestamp = -1LL;
        }
        for (int i = 0; i < SEVEN_DAY_INTERVAL_COUNT; i++)
        {
            _7_days[ts_index(m, i, SEVEN_DAY_INTERVAL_COUNT)].timestamp = -1LL;
        }
        for (int i = 0; i < NINETY_DAY_INTERVAL_COUNT; i++)
        {
            _90_days[ts_index(m, i, NINETY_DAY_INTERVAL_COUNT)].timestamp = -1LL;
        }
    }

    app_log(LOG_INFO, MR_TAG, "Element size = %d", sizeof(timeseries_data_t));
    app_log(LOG_INFO, MR_TAG, "Allocated = %d",
            (UNBUCKETED_COUNT * sizeof(timeseries_data_t)) +
                _size_highs_and_lows_all_time +
                _size_highs_and_lows_24_hours +
                _size_24_hours + _size_7_days +
                _size_90_days);
    app_log(LOG_INFO, MR_TAG, "Storage Required = %d (24h=%d, 7d=%d, 90d=%d, HL=%d)",
            _size_24_hours + _size_7_days + _size_90_days + _size_highs_and_lows_all_time,
            _size_24_hours,
            _size_7_days,
            _size_90_days,
            _size_highs_and_lows_all_time);

    edt_add_system_event_handler(SYSTEM_EVENT_TIME_UPDATED, [this](system_event_t ev)
                                 {
                                     time_t t = time(NULL) + TIME_ZONE_OFFSET;

                                     struct tm timeinfo;

                                     localtime_r(&t, &timeinfo);

                                     _last_hour = timeinfo.tm_hour;
                                     _last_5_min = timeinfo.tm_min / 5;
                                     _last_day = timeinfo.tm_wday;

                                     _time_sync_complete = true; //
                                 });

    xTaskCreateWithCaps(storage_task, "storage_task", 8192, NULL, 1, &_storage_task_handle, MALLOC_CAP_SPIRAM);
}

void MetricsRepository::accumulate(timeseries_data_t *ts, accumulation_t *acc)
{
    metric_id_t metric_id = ts->metric_id;
    acc->count[metric_id]++;
    acc->values[metric_id] += ts->average;

    if ((acc->min_set[metric_id] == false) || (ts->min < acc->min[metric_id]))
    {
        acc->min[metric_id] = ts->min;
        acc->min_set[metric_id] = true;
    }
    if ((acc->max_set[metric_id] == false) || (ts->max > acc->max[metric_id]))
    {
        acc->max[metric_id] = ts->max;
        acc->max_set[metric_id] = true;
    }
}

void MetricsRepository::accumulate_interval(time_t interval_start,
                                            time_t interval_end,
                                            timeseries_data_t *ts,
                                            int interval_count,
                                            int head_pos,
                                            accumulation_t *acc)
{
    for (metric_id_t m = (metric_id_t)0; m < METRIC_COUNT; m = (metric_id_t)(m + 1))
    {
        for (int offset = 0; offset < interval_count; offset++)
        {
            int head_pos2 = head_pos - (offset + 1);

            if (head_pos2 < 0)
            {
                head_pos2 += interval_count;
            }

            timeseries_data_t *ts2 = &ts[ts_index(m, head_pos2, interval_count)];

            if (ts2->timestamp < 0L)
            {
                break;
            }

            if (ts2->timestamp >= interval_start && ts2->timestamp < interval_end)
            {
                accumulate(ts2, acc);
            }
        }
    }
}

void MetricsRepository::distribute(time_t timestamp, timeseries_data_t *ts, int interval_count, int head_pos, accumulation_t *acc)
{
    for (metric_id_t m = (metric_id_t)0; m < METRIC_COUNT; m = (metric_id_t)(m + 1))
    {
        timeseries_data_t &ts2 = ts[ts_index(m, head_pos, interval_count)];

        ts2 = {};
        ts2.timestamp = timestamp;
        ts2.sensor_id = _metric_map[m];
        ts2.metric_id = (metric_id_t)m;

        if (acc->count[m] == 0)
        {
            ts2.time_series_attributes = TS_IS_VALID | TS_NO_VALUE,
            ts2.max = 0.0f;
            ts2.min = 0.0f;
            ts2.average = 0.0f;
        }
        else
        {
            ts2.time_series_attributes = TS_IS_VALID;
            ts2.max = acc->max[m];
            ts2.min = acc->min[m];
            ts2.average = acc->values[m] / acc->count[m];
        }
    }
}

void MetricsRepository::sync_5_min_inteval(time_t now)
{
    timeseries_data_t *in_next_bucket = _unbucketed;
    timeseries_data_t *processing = _unbucketed;

    accumulation_t accumulation = {};

    bool all_time_low_dirty = false;
    bool all_time_high_dirty = false;

    time_t interval_end = now - (now % 300);
    time_t interval_start = interval_end - 300;

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
            accumulate(curr, &accumulation);

            int li = ts_index(curr->metric_id, LOW_VALUE, HIGH_LOW_COUNT);
            int hi = ts_index(curr->metric_id, HIGH_VALUE, HIGH_LOW_COUNT);

            if (_highs_and_lows_all_time[li].timestamp < 0 || _highs_and_lows_24_hours[li].min > curr->min)
            {
                _highs_and_lows_all_time[li].timestamp = curr->timestamp;
                _highs_and_lows_all_time[li].time_series_attributes = TS_IS_VALID;
                _highs_and_lows_all_time[li].min = curr->min;
                _highs_and_lows_all_time[li].max = curr->max;
                _highs_and_lows_all_time[li].average = curr->average;

                all_time_low_dirty = true;
            }
            if (_highs_and_lows_all_time[hi].timestamp < 0 || _highs_and_lows_24_hours[hi].max < curr->max)
            {
                _highs_and_lows_all_time[hi].timestamp = curr->timestamp;
                _highs_and_lows_all_time[hi].time_series_attributes = TS_IS_VALID;
                _highs_and_lows_all_time[hi].min = curr->min;
                _highs_and_lows_all_time[hi].max = curr->max;
                _highs_and_lows_all_time[hi].average = curr->average;

                all_time_high_dirty = true;
            }
        }
    }

    distribute(interval_start, _24_hours, _24_hour_head, ONE_DAY_INTERVAL_COUNT, &accumulation);

    _unbucketed_head = in_next_bucket - _unbucketed;

    save_metrics(_24_hr_file, _24_hours, _24_hour_head, ONE_DAY_INTERVAL_COUNT);

    if (all_time_low_dirty)
    {
        save_metrics(_lh_file, _highs_and_lows_all_time, LOW_VALUE, HIGH_LOW_COUNT);
    }

    if (all_time_high_dirty)
    {
        save_metrics(_lh_file, _highs_and_lows_all_time, HIGH_VALUE, HIGH_LOW_COUNT);
    }

    _24_hour_head = (_24_hour_head + 1) % ONE_DAY_INTERVAL_COUNT;
}

void MetricsRepository::sync_1_hour_interval(time_t now)
{
    time_t interval_end = now - (now % 3600);
    time_t interval_start = interval_end - 3600;

    accumulation_t accumulation = {};

    accumulate_interval(interval_start, interval_end, _24_hours, ONE_DAY_INTERVAL_COUNT, _24_hour_head, &accumulation);

    distribute(interval_start, _7_days, SEVEN_DAY_INTERVAL_COUNT, _7_day_head, &accumulation);

    save_metrics(_7_day_file, _7_days, _7_day_head, SEVEN_DAY_INTERVAL_COUNT);

    _7_day_head = (_7_day_head + 1) % SEVEN_DAY_INTERVAL_COUNT;
}

void MetricsRepository::sync_1_day_interval(time_t now)
{
    time_t interval_end = now - (now % 86400);
    time_t interval_start = interval_end - 86400;

    accumulation_t accumulation = {};

    accumulate_interval(interval_start, interval_end, _7_days, SEVEN_DAY_INTERVAL_COUNT, _7_day_head, &accumulation);

    distribute(interval_start, _90_days, NINETY_DAY_INTERVAL_COUNT, _90_day_head, &accumulation);

    save_metrics(_90_day_file, _90_days, _90_day_head, NINETY_DAY_INTERVAL_COUNT);

    _90_day_head = (_90_day_head + 1) % NINETY_DAY_INTERVAL_COUNT;
}

void MetricsRepository::save_metrics(const char *file_name, void *metrics, int row, int interval_count)
{
    FILE *f = fopen(file_name, "r+b");

    if (f == NULL)
    {
        ESP_LOGE(MR_TAG, "Could not save metrics at row %d in file %s", row, file_name);

        return;
    }

    long seek_pos = row * interval_count * sizeof(timeseries_data_t);
    size_t size = sizeof(timeseries_data_t) * METRIC_COUNT;

    int err = fseek(f, seek_pos, SEEK_SET);

    if (err != 0)
    {
        ESP_LOGE(MR_TAG, "Could not save metrics at row %d in file %s [seek to %ld]", row, file_name, seek_pos);

        fclose(f);

        return;
    }

    void *write_from = metrics;

    size_t written = fwrite(write_from, 1, size, f);

    fflush(f);
    fclose(f);

    if (written != size)
    {
        ESP_LOGE(MR_TAG, "Failed to write enough bytes at row %d in file %s [seek to %ld], wrote %ld, expected %ld", row, file_name, seek_pos, written, size);
    }
}

void MetricsRepository::sync_metrics()
{
    time_t t = time(NULL) + TIME_ZONE_OFFSET;

    struct tm timeinfo;

    localtime_r(&t, &timeinfo);

    int five_min = timeinfo.tm_min / 5;
    int hour = timeinfo.tm_hour;
    int day = timeinfo.tm_wday;

    if (five_min != _last_5_min)
    {
        sync_5_min_inteval(t);

        _last_5_min = five_min;
    }

    if (hour != _last_hour)
    {
        sync_1_hour_interval(t);

        _last_hour = hour;
    }

    if (day != _last_day)
    {
        sync_1_day_interval(t);

        clear_24_hr_hl();

        _last_day = day;
    }
}

void MetricsRepository::clear_24_hr_hl()
{
    for (metric_id_t m = (metric_id_t)0; m < METRIC_COUNT; m = (metric_id_t)(m + 1))
    {
        for (metric_hl_t hl = (metric_hl_t)0; hl < HIGH_LOW_COUNT; hl = (metric_hl_t)(hl + 1))
        {
            _highs_and_lows_24_hours[ts_index(m, hl, HIGH_LOW_COUNT)].timestamp = -1L;
        }
    }
}

void MetricsRepository::load_all()
{
    load_metrics_file(_lh_file, (void *)_highs_and_lows_all_time, _size_highs_and_lows_all_time);
    load_metrics_file(_24_hr_file, (void *)_24_hours, _size_24_hours);
    load_metrics_file(_7_day_file, (void *)_7_days, _size_7_days);
    load_metrics_file(_90_day_file, (void *)_90_days, _size_90_days);

    _24_hour_head = find_head(_24_hours, ONE_DAY_INTERVAL_COUNT);
    _7_day_head = find_head(_7_days, SEVEN_DAY_INTERVAL_COUNT);
    _90_day_head = find_head(_90_days, NINETY_DAY_INTERVAL_COUNT);
}

int MetricsRepository::find_head(timeseries_data_t *ts, int interval_count)
{
    int head = 0;
    time_t min_time = -1L;

    for (int i = 0; i < interval_count; i++)
    {
        time_t timestamp = ts[ts_index(METRIC_INSIDE_BME280_TEMPERATURE, i, interval_count)].timestamp;

        if (timestamp < 0L)
        {
            return i;
        }
        else if (min_time < 0L || timestamp < min_time)
        {
            min_time = timestamp;
            head = i;
        }
    }

    return head;
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

    _latest[id].timestamp = time(NULL) + TIME_ZONE_OFFSET;
    _latest[id].average = value;
    _latest[id].min = value;
    _latest[id].max = value;

    int index = ts_index(id, HIGH_VALUE, HIGH_LOW_COUNT);

    if (_highs_and_lows_24_hours[index].timestamp < 0 || _highs_and_lows_24_hours[index].max < value)
    {
        _highs_and_lows_24_hours[index].average = value;
        _highs_and_lows_24_hours[index].min = value;
        _highs_and_lows_24_hours[index].max = value;
        _highs_and_lows_24_hours[index].timestamp = _latest[id].timestamp;

        attributes |= IS_24_HR_HIGH;
    }
    if (_highs_and_lows_24_hours[index].timestamp < 0 || _highs_and_lows_24_hours[index].min > value)
    {
        _highs_and_lows_24_hours[index].average = value;
        _highs_and_lows_24_hours[index].min = value;
        _highs_and_lows_24_hours[index].max = value;
        _highs_and_lows_24_hours[index].timestamp = _latest[id].timestamp;

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
