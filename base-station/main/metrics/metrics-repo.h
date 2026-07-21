#ifndef Db_h
#define Db_h

#include <stdint.h>
#include <time.h>
#include "models.h"
#include "events/edt.h"
#include "esp_task.h"

#define ONE_DAY_INTERVAL_COUNT (24 * 12)
#define SEVEN_DAY_INTERVAL_COUNT (7 * 24)
#define NINETY_DAY_INTERVAL_COUNT (90)

typedef enum : uint16_t
{
    HIGH_VALUE,
    LOW_VALUE,
    HIGH_LOW_COUNT
} metric_hl_t;

typedef struct
{
    float min[METRIC_COUNT]{};
    float max[METRIC_COUNT]{};
    float values[METRIC_COUNT]{};
    int count[METRIC_COUNT]{};
    bool min_set[METRIC_COUNT]{};
    bool max_set[METRIC_COUNT]{};
} accumulation_t;

class MetricsRepository
{
public:
    timeseries_data_t get_latest(sensor_id_t sensor_id, metric_id_t metric_id)
    {
        return _latest[metric_id];
    }

    void init();

    static void handle_sensor_data(sensor_data_t sensor_data);

private:
    typedef void (MetricsRepository::*Func)(sensor_data_t *);

    void update(sensor_data_t *sensor_data);

    inline int ts_index(metric_id_t metric_id, int row, int interval_count)
    {
        return (row * interval_count) + metric_id;
    }

    void load_all();
    void load_metrics_file(const char *file_name, void *destination, size_t size);
    void reset_file(const char *file_name, void *destination, size_t size);

    void accumulate(timeseries_data_t *ts, accumulation_t *acc);

    void accumulate_interval(time_t interval_start,
                             time_t interval_end,
                             timeseries_data_t *ts,
                             int interval_count,
                             int head_pos,
                             accumulation_t *acc);

    void distribute(time_t timestamp, timeseries_data_t *ts, int interval_count, int head_pos, accumulation_t *acc);

    void save_metrics(const char *file_name, void *metrics, int row, int interval_count);

    void sync_metrics();

    void sync_5_min_inteval(time_t now);
    void sync_1_hour_interval(time_t now);
    void sync_1_day_interval(time_t now);

    void sync_90_day_hl();

    void clear_24_hr_hl();

    int find_head(timeseries_data_t *ts, int interval_count);

    void update_time_series_data(sensor_data_t *sensor_data, metric_id_t metric_id, float value);

    float calculate_inside_aqi();
    float calculate_uvi();
    float calculate_rainfall(int in_last_seconds);

    void update_inside_bme280(sensor_data_t *sensor_data);
    void update_inside_sgp30(sensor_data_t *sensor_data);
    void update_outside1_bme280(sensor_data_t *sensor_data);
    void update_outside_uv(sensor_data_t *sensor_data);
    void update_outside_sps30_set1(sensor_data_t *sensor_data);
    void update_outside_sps30_set2(sensor_data_t *sensor_data);
    void update_outside_sps30_set3(sensor_data_t *sensor_data);
    void update_outside_rain_gauge(sensor_data_t *sensor_data);

    Func _sensor_updates[9] = {
        &MetricsRepository::update_inside_bme280,
        &MetricsRepository::update_inside_sgp30,
        &MetricsRepository::update_outside1_bme280,
        &MetricsRepository::update_outside_uv,
        &MetricsRepository::update_outside_sps30_set1,
        &MetricsRepository::update_outside_sps30_set2,
        &MetricsRepository::update_outside_sps30_set3,
        &MetricsRepository::update_outside_rain_gauge,
    };

    timeseries_data_t *_latest;
    timeseries_data_t *_unbucketed;
    timeseries_data_t *_highs_and_lows_24_hours;
    timeseries_data_t *_highs_and_lows_all_time;
    timeseries_data_t *_24_hours;
    timeseries_data_t *_7_days;
    timeseries_data_t *_90_days;

    TaskHandle_t _storage_task_handle = NULL;

    volatile bool _time_sync_complete = false;

    int _unbucketed_head;
    int _24_hour_head;
    int _7_day_head;
    int _90_day_head;

    int _last_5_min;
    int _last_hour;
    int _last_day;

    static MetricsRepository *_instance;

    const size_t _size_highs_and_lows_all_time = sizeof(timeseries_data_t) * HIGH_LOW_COUNT * METRIC_COUNT;
    const size_t _size_highs_and_lows_24_hours = sizeof(timeseries_data_t) * HIGH_LOW_COUNT * METRIC_COUNT;
    const size_t _size_24_hours = sizeof(timeseries_data_t) * ONE_DAY_INTERVAL_COUNT * METRIC_COUNT;
    const size_t _size_7_days = sizeof(timeseries_data_t) * SEVEN_DAY_INTERVAL_COUNT * METRIC_COUNT;
    const size_t _size_90_days = sizeof(timeseries_data_t) * NINETY_DAY_INTERVAL_COUNT * METRIC_COUNT;

    friend void storage_task(void *);

    const sensor_id_t _metric_map[METRIC_COUNT - 1] = {
        SENSOR_INSIDE_BME280,
        SENSOR_INSIDE_BME280,
        SENSOR_INSIDE_BME280,
        SENSOR_INSIDE_SGP30,
        SENSOR_INSIDE_SGP30,
        SENSOR_INSIDE_SGP30,
        SENSOR_OUTSIDE1_BME280,
        SENSOR_OUTSIDE1_BME280,
        SENSOR_OUTSIDE1_BME280,
        SENSOR_OUTSIDE1_BME280,
        SENSOR_OUTSIDE_UV,
        SENSOR_OUTSIDE_UV,
        SENSOR_OUTSIDE_UV,
        SENSOR_OUTSIDE_SPS30_SET1,
        SENSOR_OUTSIDE_SPS30_SET1,
        SENSOR_OUTSIDE_SPS30_SET1,
        SENSOR_OUTSIDE_SPS30_SET2,
        SENSOR_OUTSIDE_SPS30_SET2,
        SENSOR_OUTSIDE_SPS30_SET2,
        SENSOR_OUTSIDE_SPS30_SET3,
        SENSOR_OUTSIDE_SPS30_SET3,
        SENSOR_OUTSIDE_SPS30_SET3,
        SENSOR_OUTSIDE_RAIN_GAUGE,
        SENSOR_OUTSIDE_RAIN_GAUGE,
        SENSOR_OUTSIDE_RAIN_GAUGE,
        SENSOR_OUTSIDE_RAIN_GAUGE,
        SENSOR_OUTSIDE_RAIN_GAUGE,
        SENSOR_OUTSIDE_RAIN_GAUGE,
    };
};

#endif
