#ifndef Forecast_Task_h
#define Forecast_Task_h

#include <stdint.h>
#include "esp_err.h"

typedef struct
{
    char date[11];
    int16_t weather_code;
    float temp_min;
    float temp_max;
    float pop;
    float rain;
    float snow;
    float wind_speed;
    float wind_gust;
    int16_t wind_dir;
    float uv_index;
} forecast_day_t;

typedef struct
{
    forecast_day_t current;
    forecast_day_t today;
    forecast_day_t tomorrow;
} forecast_t;

#ifdef __cplusplus
extern "C"
{
#endif
    extern esp_err_t forecast_init();

    extern esp_err_t get_forecast(forecast_t *fc);

#ifdef __cplusplus
}
#endif

#endif
