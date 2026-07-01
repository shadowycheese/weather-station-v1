#ifndef Db_h
#define Db_h

#include <stdint.h>
#include <time.h>
#include "models.h"

typedef enum : uint16_t
{
    DATA_BATTERY,
    DATA_TEMPERATURE,
    DATA_PRESSURE,
    DATA_HUMIDITY,
    DATA_AQ_VOC,
    DATA_AQ_PMX,
    DATA_UV,
    DATA_RAIN,
    DATA_MAX
} data_type_t;

typedef struct
{
    time_t timestamp;
    sensor_id_t sensor_id;
    data_type_t data_type;
    float max;
    float min;
    float average;
} __attribute__((packed)) timeseries_data_t;

class Db
{
public:
    Db();

    timeseries_data_t get_latest(sensor_id_t sensor_id, data_type_t data_type)
    {
        return _latest[sensor_id][data_type];
    }

    void update(sensor_data_t sensor_data);

private:
    timeseries_data_t to_time_series_data(sensor_data_t sensor_data, data_type_t data_type, float value);

    timeseries_data_t _latest[SENSOR_COUNT][DATA_MAX];
    timeseries_data_t *_unbucketed;
    timeseries_data_t *_24_hours;
    timeseries_data_t *_7_days;
    timeseries_data_t *_90_days;
};

#endif
