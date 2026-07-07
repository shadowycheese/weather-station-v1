#include "esp_heap_caps.h"
#include "events/edt.h"
#include "metrics/metrics-repo.h"
#include "log/debug.h"

MetricsRepository *MetricsRepository::_instance = NULL;

#define ONE_HOUR 3600
#define SIX_HOURS (6 * ONE_HOUR)
#define TWELVE_HOURS (12 * ONE_HOUR)
#define ONE_DAY (24 * ONE_HOUR)

MetricsRepository::MetricsRepository()
{
    _instance = this;
}

void MetricsRepository::init()
{
    _latest = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * METRIC_COUNT, MALLOC_CAP_SPIRAM);
    _24h_high = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * METRIC_COUNT, MALLOC_CAP_SPIRAM);
    _24h_low = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * METRIC_COUNT, MALLOC_CAP_SPIRAM);

    for (int i = 0; i < METRIC_COUNT; i++)
    {
        _24h_high[i].timestamp = -1LL;
        _24h_high[i].sensor_id = _metric_map[i];
        _24h_high[i].metric_id = (metric_id_t)i;

        _24h_low[i].timestamp = -1LL;
        _24h_low[i].sensor_id = _metric_map[i];
        _24h_low[i].metric_id = (metric_id_t)i;

        _latest[i].timestamp = -1LL;
        _latest[i].sensor_id = _metric_map[i];
        _latest[i].metric_id = (metric_id_t)i;
    }

    //_unbucketed = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * 10000, MALLOC_CAP_SPIRAM);
    //_24_hours = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * 6000, MALLOC_CAP_SPIRAM);
    //_7_days = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * 500, MALLOC_CAP_SPIRAM);
    //_90_days = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * 2000, MALLOC_CAP_SPIRAM);
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

    if (_24h_high[id].max < value)
    {
        _24h_high[id].average = value;
        _24h_high[id].min = value;
        _24h_high[id].max = value;
        _24h_high[id].timestamp = _latest[id].timestamp;

        attributes |= IS_24_HR_HIGH;
    }
    if (_24h_low[id].min > value)
    {
        _24h_low[id].average = value;
        _24h_low[id].min = value;
        _24h_low[id].max = value;
        _24h_low[id].timestamp = _latest[id].timestamp;

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