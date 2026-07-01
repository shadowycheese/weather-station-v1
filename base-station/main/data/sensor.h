#ifndef Sensor_h
#define Sensor_h

#include "espspi.h"
#include "data/db.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern void sensor_read_start(Db *db);

#ifdef __cplusplus
}
#endif

#endif