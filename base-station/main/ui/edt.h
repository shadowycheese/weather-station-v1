#ifndef EDT_h
#define EDT_h

#include <functional>
#include "esp_log.h"
#include "metrics.h"
#include "models.h"

#define IS_24_HR_LOW 0x0001
#define IS_24_HR_HIGH 0x0002

typedef enum
{
    JOB_TYPE_METRIC,
    JOB_TYPE_LAMBDA,
} job_type_t;

typedef struct
{
    job_type_t type;
    std::function<void()> execute;
    metric_event_t metric_event;
} edt_job_t;

using metric_event_handler_t = std::function<void(metric_event_t)>;

#ifdef __cplusplus
extern "C"
{
#endif
    extern void edt_init();
    extern void edt_registier(metric_id_t metric_id, metric_event_handler_t callback);
    extern void edt_post(edt_job_t job);

#ifdef __cplusplus
}
#endif

#endif