#ifndef Forecast_h
#define Forecast_h

#include "lv.hpp"
#include "models.h"
#include "events/edt.h"
#include "ui/core/card.h"
#include "ui/ui-helpers.h"
#include "net/forecast-task.h"
#include "theme.h"

class ForecastDay
{
public:
    void init(lv::ObjectView parent);
    void update(forecast_day_t *fc);

    void clear();

private:
    lv::Spangroup _forecast;
    lv_span_t *_date;
    lv_span_t *_temp_min;
    lv_span_t *_temp_max;
    lv_span_t *_weather;
    lv_span_t *_wind;
    lv_span_t *_wind_dir;
    lv_span_t *_pop;
    lv_span_t *_snow;
    lv_span_t *_rain;

    const char *weather_code_icon(int16_t wc);
    const char *wind_dir(int16_t wd);
    lv_color_t weather_code_color(int16_t wc);
    lv_color_t wind_speed_color(float windspeed);
    lv_color_t pop_color(float windspeed);
};

class Forecast : public Card
{
public:
    virtual void init(lv::ObjectView parent)
    {
        auto box = create_vbox(parent, SCREEN_WIDTH, 120).bg_color(COL_BACKGROUND_2).padding(6);

        _today.init(box);
        _tomorrow.init(box);

        edt_add_system_event_handler(SYSTEM_EVENT_FORECAST, [this](system_event_t m)
                                     {
                                         forecast_t fc;

                                         printf("Got forecast event");

                                         if (get_forecast(&fc) == ESP_OK)
                                         {
                                             _today.update(&fc.today);
                                             _tomorrow.update(&fc.tomorrow);
                                         }
                                         else
                                         {
                                             _today.clear();
                                             _tomorrow.clear();
                                         } //
                                     });
    }

private:
    ForecastDay _today;
    ForecastDay _tomorrow;
};

#endif