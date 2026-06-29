#ifndef EDT_h
#define EDT_h

#include <functional>
#include "models.h"
#include "display.h"

typedef enum
{
    JOB_TYPE_SENSOR_DATA,
    JOB_TYPE_LAMBDA,
} job_type_t;

typedef struct
{
    job_type_t type;
    std::function<void()> execute;
    union
    {
        sensor_data_t sensor_data;
    } payload;
} edt_job_t;

#ifdef __cplusplus
extern "C"
{
#endif
    extern inline Display *active_display();

    extern void edt_init(Display *_display);
    extern void edt_post(edt_job_t job);

#ifdef __cplusplus
}
#endif

#endif