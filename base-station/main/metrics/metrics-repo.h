#ifndef Db_h
#define Db_h

#include <stdint.h>
#include <time.h>
#include "models.h"
#include "events/edt.h"

class MetricsRepository
{
public:
    MetricsRepository();

    timeseries_data_t get_latest(sensor_id_t sensor_id, metric_id_t metric_id)
    {
        return _latest[metric_id];
    }

    void init();

    static void handle_sensor_data(sensor_data_t sensor_data);

private:
    typedef void (MetricsRepository::*Func)(sensor_data_t *);

    void update(sensor_data_t *sensor_data);

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
    timeseries_data_t *_24h_high;
    timeseries_data_t *_24h_low;
    timeseries_data_t *_unbucketed;
    timeseries_data_t *_24_hours;
    timeseries_data_t *_7_days;
    timeseries_data_t *_90_days;

    static MetricsRepository *_instance;

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
