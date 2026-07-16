#ifndef System_Events_H
#define System_Events_H

#include <stdint.h>
#include <time.h>

#define TIME_ZONE_OFFSET (-(7L * 3600L))

#include "models.h"

typedef enum : uint16_t
{
    SYSTEM_EVENT_TICK,
    SYSTEM_EVENT_TIME_UPDATED,
    SYSTEM_EVENT_WIFI_STATUS,
    SYSTEM_EVENT_FORECAST,
    SYSTEM_EVENT_COUNT,
} system_event_id_t;

typedef struct
{
    system_event_id_t event_id;
    int16_t attributes;
    int32_t value;
} system_event_t;

#endif