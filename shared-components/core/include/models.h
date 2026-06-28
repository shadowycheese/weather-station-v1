#ifndef MODELS_H
#define MODELS_H

#include <stdint.h>

typedef struct
{
    uint16_t node_id;
    float reading1;
    float reading2;
    float reading3;
    uint16_t battery_mv;
} __attribute__((packed)) sensor_data_t;

#endif