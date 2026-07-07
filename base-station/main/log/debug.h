#ifndef _Debug__h
#define _Debug__h

#include "models.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void debug_sensor(const char *tag, sensor_data_t *sd);
#ifdef __cplusplus
}
#endif

#endif