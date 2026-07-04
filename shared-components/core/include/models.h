#ifndef MODELS_H
#define MODELS_H

#include <stdint.h>

typedef enum : uint16_t
{
    SENSOR_TEMP_INSIDE,
    SENSOR_TEMP_OUTSIDE1,
    SENSOR_TEMP_OUTSIDE2,
    SENSOR_AQ_INSIDE,
    SENSOR_AQ_OUTSIDE,
    SENSOR_UV,
    SENSOR_RAIN_GAUGE,
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