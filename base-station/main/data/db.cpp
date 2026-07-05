#include "db.h"
#include "esp_heap_caps.h"
#include "edt.h"

Db *Db::_instance = NULL;

#define ONE_HOUR 3600
#define SIX_HOURS (6 * ONE_HOUR)
#define TWELVE_HOURS (12 * ONE_HOUR)
#define ONE_DAY (24 * ONE_HOUR)

Db::Db()
{
    _instance = this;

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
}

void Db::init()
{
    //_unbucketed = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * 10000, MALLOC_CAP_SPIRAM);
    //_24_hours = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * 6000, MALLOC_CAP_SPIRAM);
    //_7_days = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * 500, MALLOC_CAP_SPIRAM);
    //_90_days = (timeseries_data_t *)heap_caps_malloc(sizeof(timeseries_data_t) * 2000, MALLOC_CAP_SPIRAM);
}

void Db::handle_sensor_data(sensor_data_t sensor_data)
{
    _instance->update(&sensor_data);
}

void Db::update(sensor_data_t *sd)
{
    metric_event_t events[6];
    int ec = 0;
    sensor_id_t id = sd->sensor_id;
    switch (id)
    {
    case SENSOR_INSIDE_BME280:
        events[ec++] = update_time_series_data(sd, METRIC_INSIDE_BME280_TEMPERATURE, sd->reading1);
        events[ec++] = update_time_series_data(sd, METRIC_INSIDE_BME280_HUMIDITY, sd->reading2);
        events[ec++] = update_time_series_data(sd, METRIC_INSIDE_BME280_PRESSURE, sd->reading3);
        break;
    case SENSOR_INSIDE_SGP30:
        events[ec++] = update_time_series_data(sd, METRIC_INSIDE_SGP30_ECO2, sd->reading1);
        events[ec++] = update_time_series_data(sd, METRIC_INSIDE_SGP30_TVOC, sd->reading2);
        events[ec++] = update_time_series_data(sd, METRIC_INSIDE_SGP30_AQI, calculate_inside_aqi());
        break;
    case SENSOR_OUTSIDE1_BME280:
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE1_BME280_TEMPERATURE, sd->reading1);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE1_BME280_HUMIDITY, sd->reading2);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE1_BME280_PRESSURE, sd->reading3);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE1_BME280_BATTERY, sd->battery_mv);
        break;
    case SENSOR_OUTSIDE_UV:
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_UV_RAW, sd->reading1);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_UV_INDEX, calculate_uvi());
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_UV_BATTERY, sd->battery_mv);
        break;
    case SENSOR_OUTSIDE_SPS30_SET1:
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM1P0MASS, sd->reading1);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM2P5MASS, sd->reading2);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM4P0MASS, sd->reading3);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_BATTERY, sd->battery_mv);
        break;
    case SENSOR_OUTSIDE_SPS30_SET2:
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM10MASS, sd->reading1);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM0P5NUMBER, sd->reading2);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM1P0NUMBER, sd->reading3);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_BATTERY, sd->battery_mv);
        break;
    case SENSOR_OUTSIDE_SPS30_SET3:
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM2P5NUMBER, sd->reading1);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM4P0NUMBER, sd->reading2);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_PM10NUMBER, sd->reading3);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_SPS30_BATTERY, sd->battery_mv);
        break;
    case SENSOR_OUTSIDE_RAIN_GAUGE:
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_TICKS, sd->reading1);
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_MM_HOUR, calculate_rainfall(ONE_HOUR));
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_MM_6HOURS, calculate_rainfall(SIX_HOURS));
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_MM_12HOURS, calculate_rainfall(TWELVE_HOURS));
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_MM_24HOURS, calculate_rainfall(ONE_DAY));
        events[ec++] = update_time_series_data(sd, METRIC_OUTSIDE_RAINFULL_BATTERY, sd->battery_mv);
        break;
    default:
        break;
    }

    for (int i = 0; i < ec; i++)
    {
        edt_job_t job = {
            .type = JOB_TYPE_METRIC,
            .metric_event = events[i],
        };

        edt_post(job);
    }
}

float Db::calculate_uvi()
{
    return 0;
}

float Db::calculate_inside_aqi()
{
    return 0;
}

float Db::calculate_rainfall(int in_last_seconds)
{
    return 0;
}

metric_event_t Db::update_time_series_data(sensor_data_t *sensor_data, metric_id_t metric_id, float value)
{
    uint16_t attributes = 0;

    _latest[metric_id].timestamp = time(NULL);
    _latest[metric_id].average = value;
    _latest[metric_id].min = value;
    _latest[metric_id].max = value;

    if (_24h_high[metric_id].max < value)
    {
        _24h_high[metric_id].average = value;
        _24h_high[metric_id].min = value;
        _24h_high[metric_id].max = value;
        _24h_high[metric_id].timestamp = _latest[metric_id].timestamp;

        attributes |= IS_24_HR_HIGH;
    }
    if (_24h_low[metric_id].min > value)
    {
        _24h_low[metric_id].average = value;
        _24h_low[metric_id].min = value;
        _24h_low[metric_id].max = value;
        _24h_low[metric_id].timestamp = _latest[metric_id].timestamp;

        attributes |= IS_24_HR_LOW;
    }

    metric_event_t event = {
        .metric_id = metric_id,
        .attributes = attributes,
        .value = value,
    };

    return event;
}