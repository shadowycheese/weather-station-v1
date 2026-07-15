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

    void load_all();
    void load_metrics_file(const char *file_name, void *destination, size_t size);
    void reset_file(const char *file_name, void *destination, size_t size);

    void save_5_minute_interval();
    void save_1_hour_interval();
    void save_1_day_minute_interval();

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

    void sync_metrics();

    void sync_5_min_inteval(time_t now);
    void sync_1_hour_interval(time_t now);
    void sync_1_day_interval(time_t now);

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
    timeseries_data_t (*_highs_and_lows)[HIGH_LOW_COUNT];
    timeseries_data_t (*_24_hours)[ONE_DAY_INTERVAL_COUNT];
    timeseries_data_t (*_7_days)[SEVEN_DAY_INTERVAL_COUNT];
    timeseries_data_t (*_90_days)[NINETY_DAY_INTERVAL_COUNT];

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

    const size_t _size_highs_and_lows = sizeof(*_highs_and_lows) * METRIC_COUNT;
    const size_t _size_24_hours = sizeof(*_24_hours) * METRIC_COUNT;
    const size_t _size_7_days = sizeof(*_7_days) * METRIC_COUNT;
    const size_t _size_90_days = sizeof(*_90_days) * METRIC_COUNT;

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
