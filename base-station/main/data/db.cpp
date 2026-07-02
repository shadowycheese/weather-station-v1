#include "db.h"
#include "esp_heap_caps.h"

Db *Db::_instance = NULL;

Db::Db()
{
    _instance = this;
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

void Db::update(sensor_data_t *sensor_data)
{
    sensor_id_t id = sensor_data->sensor_id;
    switch (id)
    {
    case SENSOR_TEMP_INSIDE:
    case SENSOR_TEMP_OUTSIDE1:
    case SENSOR_TEMP_OUTSIDE2:
        _latest[id][DATA_BATTERY] = to_time_series_data(sensor_data, DATA_BATTERY, sensor_data->battery_mv);
        _latest[id][DATA_TEMPERATURE] = to_time_series_data(sensor_data, DATA_TEMPERATURE, sensor_data->reading1);
        _latest[id][DATA_HUMIDITY] = to_time_series_data(sensor_data, DATA_HUMIDITY, sensor_data->reading2);
        _latest[id][DATA_PRESSURE] = to_time_series_data(sensor_data, DATA_PRESSURE, sensor_data->reading3);
        break;
    case SENSOR_AQ_INSIDE:
        _latest[id][DATA_UV] = to_time_series_data(sensor_data, DATA_AQ_VOC, sensor_data->reading1);
        break;
    case SENSOR_AQ_OUTSIDE:
        _latest[id][DATA_UV] = to_time_series_data(sensor_data, DATA_AQ_PMX, sensor_data->reading1);
        break;
    case SENSOR_UV:
        _latest[id][DATA_UV] = to_time_series_data(sensor_data, DATA_UV, sensor_data->reading1);
        break;
    case SENSOR_RAIN_GAUGE:
        _latest[id][DATA_RAIN] = to_time_series_data(sensor_data, DATA_RAIN, sensor_data->reading1);
        break;
    default:
        break;
    }
}

timeseries_data_t Db::to_time_series_data(sensor_data_t *sensor_data, data_type_t data_type, float value)
{
    timeseries_data_t t = {
        .timestamp = time(NULL),
        .sensor_id = sensor_data->sensor_id,
        .data_type = data_type,
        .max = value,
        .min = value,
        .average = value,
    };

    return t;
}