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
    void init(const char *title, lv::ObjectView parent);
    void update(forecast_day_t *fc);

    void clear();

private:
    lv::Label _day;
    lv::Image _weather;
    lv::Label _temp;
    lv::Spangroup _pop;
    lv_span_t *_pop_pc;
    lv::Spangroup _precip;
    lv_span_t *_pr_total;
    lv_span_t *_pr_units;
    lv::Image _pr_type;
    lv::Spangroup _wind;
    lv_span_t *_wind_speed;
    lv_span_t *_wind_gust;
    lv::Image _wind_dir;
};

class Forecast : public Card
{
public:
    virtual void init(lv::ObjectView parent)
    {
        auto box = create_vbox(parent, SCREEN_WIDTH, 100).bg_color(COL_BACKGROUND_2).padding(0);

        //_today.init("Today", box);
        _tomorrow.init("Tomorrow", box);

        edt_add_system_event_handler(SYSTEM_EVENT_FORECAST, [this](system_event_t m)
                                     {
                                         forecast_t fc;

                                         if (get_forecast(&fc) == ESP_OK)
                                         {
                                             //_today.update(&fc.today);
                                             _tomorrow.update(&fc.tomorrow);
                                         }
                                         else
                                         {
                                             //_today.clear();
                                             _tomorrow.clear();
                                         } //
                                     });
    }

private:
    // ForecastDay _today;
    ForecastDay _tomorrow;
};

#endif