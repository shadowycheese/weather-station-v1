#include <stdio.h>
#include "log/debug.h"

#define DEBUG_ENABLED 1

extern "C" void debug_sensor(const char *tag, sensor_data_t *sd)
{
#if DEBUG_ENABLED
    printf("%s: %02d: %0.1f %0.1f %0.1f [%d]\n",
           tag,
           sd->sensor_id,
           sd->reading1,
           sd->reading2,
           sd->reading3,
           sd->battery_mv);
#endif
}
