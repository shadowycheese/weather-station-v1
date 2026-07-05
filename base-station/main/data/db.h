#ifndef Db_h
#define Db_h

#include <stdint.h>
#include <time.h>
#include "models.h"
#include "metrics.h"
#include "edt.h"

class Db
{
public:
    Db();

    timeseries_data_t get_latest(sensor_id_t sensor_id, metric_id_t metric_id)
    {
        return _latest[metric_id];
    }

    void init();

    static void handle_sensor_data(sensor_data_t sensor_data);

private:
    void update(sensor_data_t *sensor_data);

    metric_event_t update_time_series_data(sensor_data_t *sensor_data, metric_id_t metric_id, float value);

    float calculate_inside_aqi();

    float calculate_uvi();

    float calculate_rainfall(int in_last_seconds);

    timeseries_data_t _latest[METRIC_COUNT];
    timeseries_data_t _24h_high[METRIC_COUNT];
    timeseries_data_t _24h_low[METRIC_COUNT];
    timeseries_data_t *_unbucketed;
    timeseries_data_t *_24_hours;
    timeseries_data_t *_7_days;
    timeseries_data_t *_90_days;

    static Db *_instance;

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
