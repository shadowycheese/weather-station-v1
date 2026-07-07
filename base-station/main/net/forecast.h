#ifndef Forecast_h
#define Forecast_h

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int16_t weather_code;
        float temperature;
        float pop;
        float wind_speed;
    } forecast_day_t;

    typedef struct
    {
        forecast_day_t today;
        forecast_day_t tomorrow;
    } forecast_t;

    extern esp_err_t forecast_init();

    extern esp_err_t get_forecast(forecast_t *fc);

#ifdef __cplusplus
}
#endif

#endif
