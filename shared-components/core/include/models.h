#ifndef MODELS_H
#define MODELS_H

#include <stdint.h>

typedef enum : uint16_t
{
    SENSOR_INSIDE_BME280,
    SENSOR_INSIDE_SGP30,
    SENSOR_OUTSIDE1_BME280,
    SENSOR_OUTSIDE_UV,
    SENSOR_OUTSIDE_SPS30_SET1,
    SENSOR_OUTSIDE_SPS30_SET2,
    SENSOR_OUTSIDE_SPS30_SET3,
    SENSOR_OUTSIDE_RAIN_GAUGE,
    SENSOR_COUNT
} sensor_id_t;

typedef struct
{
    sensor_id_t sensor_id;
    float reading1;
    float reading2;
    float reading3;
    int16_t battery_mv;
} __attribute__((packed)) sensor_data_t;

#endif