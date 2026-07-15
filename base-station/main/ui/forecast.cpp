#include "ui/forecast.h"
#include "ui/extensions/style-wrapper.hpp"
#include "ui/icons/icons.h"
#include "ui/ui-helpers.h"
#include "net/forecast-task.h"

#define FORECAST_HEIGHT 100
#define DAY_WIDTH 130
#define WEATHER_ICON_WIDTH 100
#define TEMP_WIDTH 130
#define POP_WIDTH 120
#define PRECIP_WIDTH 140
#define PRECIP_ICON_WIDTH 40
#define WIND_ICON_WIDTH 40
#define WIND_WIDTH 300
#define WIND_DIR_ICON_WIDTH 40
#define GAP 4

void ForecastDay::init(const char *title, lv::ObjectView parent)
{
    auto box = create_hbox(parent, SCREEN_WIDTH, FORECAST_HEIGHT)
                   .layout_none()
                   .bg_color(COL_BACKGROUND_2);

    int x = GAP;

    _day = lv::Label::create(box)
               .text(title)
               .text_align(LV_TEXT_ALIGN_RIGHT)
               .text_color(COL_TEXT_SECONDARY)
               .width(DAY_WIDTH)
               .height(FORECAST_HEIGHT)
               .x(x)
               .y(13)
               .text_font(FONT_24PT_BOLD);

    x += DAY_WIDTH + GAP;

    _weather = lv::Image::create(box)
                   .x(x + 2)
                   .y(10)
                   .width(WEATHER_ICON_WIDTH)
                   .height(WEATHER_ICON_WIDTH);

    x += WEATHER_ICON_WIDTH;

    _temp = lv::Label::create(box)
                .text("")
                .text_align(LV_TEXT_ALIGN_CENTER)
                .text_color(COL_TEXT_PRIMARY)
                .text_font(FONT_48PT_BOLD)
                .x(x)
                .y(26)
                .width(TEMP_WIDTH)
                .height(FORECAST_HEIGHT);

    x += TEMP_WIDTH;

    int x2 = x;

    _pop = lv::Spangroup::create(box)
               .width(POP_WIDTH)
               .height(FORECAST_HEIGHT / 2)
               .text_align(LV_TEXT_ALIGN_LEFT)
               .text_color(COL_TEXT_SECONDARY)
               .text_font(FONT_24PT)
               .x(x)
               .y(10);

    auto popTitle = _pop.new_span();
    _pop_pc = _pop.new_span();
    auto popUnits = _pop.new_span();

    style_wrapper(_precip.span_style(popTitle))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT);
    style_wrapper(_precip.span_style(popUnits))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_22PT);
    style_wrapper(_precip.span_style(_pop_pc))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_30PT_BOLD);

    _pop.span_text(popTitle, "POP ");
    _pop.span_text(popUnits, "%");
    _pop.span_text(_pop_pc, "");

    x += POP_WIDTH + GAP;

    _pr_type = lv::Image::create(box)
                   .x(x)
                   .y(11)
                   .scale(64)
                   .width(PRECIP_ICON_WIDTH)
                   .height(PRECIP_ICON_WIDTH);

    x += PRECIP_ICON_WIDTH - 4;

    _precip = lv::Spangroup::create(box)
                  .width(PRECIP_WIDTH)
                  .height(FORECAST_HEIGHT / 2)
                  .text_align(LV_TEXT_ALIGN_LEFT)
                  .text_color(COL_TEXT_SECONDARY)
                  .text_font(FONT_24PT)
                  .x(x)
                  .y(10);

    auto pr_title = _precip.new_span();
    _pr_total = _precip.new_span();
    _pr_units = _precip.new_span();

    _precip.span_text(_pr_total, "");
    _precip.span_text(_pr_units, " mm");

    style_wrapper(_precip.span_style(pr_title))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT);
    style_wrapper(_precip.span_style(_pr_units))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT);
    style_wrapper(_wind.span_style(_pr_total))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_30PT_BOLD);

    x = x2;

    lv::Image::create(box)
        .src(icon_wind)
        .recolor(COL_TEXT_SECONDARY)
        .recolor_opa(LV_OPA_COVER)
        .scale(64)
        .x(x)
        .y(62)
        .width(WIND_ICON_WIDTH)
        .height(WIND_ICON_WIDTH);

    x += WIND_ICON_WIDTH - 4;

    _wind_dir = lv::Image::create(box)
                    .src(icon_arrow_up)
                    .recolor(COL_TEXT_SECONDARY)
                    .recolor_opa(LV_OPA_COVER)
                    .scale(64)
                    .x(x)
                    .y(62)
                    .width(WIND_DIR_ICON_WIDTH)
                    .height(WIND_DIR_ICON_WIDTH);

    x += WIND_DIR_ICON_WIDTH - 4;

    _wind = lv::Spangroup::create(box)
                .width(WIND_WIDTH)
                .margin_top(-7)
                .text_align(LV_TEXT_ALIGN_LEFT)
                .text_color(COL_TEXT_SECONDARY)
                .text_font(FONT_24PT)
                .x(x)
                .y(62);

    _wind_speed = _wind.new_span();
    auto wind_sep = _wind.new_span();
    _wind_gust = _wind.new_span();
    auto wind_units = _wind.new_span();

    style_wrapper(_wind.span_style(wind_sep))
        .text_color(COL_TEXT_MUTED)
        .text_font(FONT_20PT);
    style_wrapper(_wind.span_style(wind_units))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT);
    style_wrapper(_wind.span_style(_wind_speed))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_30PT_BOLD);
    style_wrapper(_wind.span_style(_wind_gust))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_30PT_BOLD);

    _wind.span_text(wind_sep, " | ");
    _wind.span_text(wind_units, " km/h");
    _wind.span_text(_wind_speed, "");
    _wind.span_text(_wind_gust, "");

    x += WIND_WIDTH + GAP;

    clear();
}

void ForecastDay::clear()
{
    _pop.span_text(_pop_pc, "");
    _precip.span_text(_pr_total, "");
    _wind.span_text(_wind_speed, "");
    _wind.span_text(_wind_gust, "");

    _precip.refr_mode();
    _pop.refr_mode();
    _wind.refr_mode();
}

void ForecastDay::update(forecast_day_t *fc)
{
    char temp[10], wind[10], gust[10], pop[10], precip[10];

    snprintf(temp, 10, "%0.1f", fc->temp_max);
    snprintf(wind, 10, "%0.1f", fc->wind_speed);
    snprintf(gust, 10, "%0.1f", fc->wind_gust);
    snprintf(pop, 10, " %0.0f", fc->pop);

    if (fc->snow > fc->rain)
    {
        snprintf(precip, 10, " %0.1f", fc->snow + (fc->rain / 10));

        _precip.span_text(_pr_units, " cm");
        _precip.span_text(_pr_total, precip);

        _pr_type.src(icon_snowflakes);
    }
    else
    {
        snprintf(precip, 10, " %0.1f", fc->rain + (fc->snow * 10));

        _precip.span_text(_pr_units, " mm");
        _precip.span_text(_pr_total, precip);

        _pr_type.src(icon_raindrops);
    }

    _pop.span_text(_pop_pc, pop);
    _wind.span_text(_wind_speed, wind);
    _wind.span_text(_wind_gust, gust);
    _temp.text(temp);
    _weather.src(weather_code_icon(fc));

    _wind_dir.rotation((fc->wind_dir + 180) * 10).invalidate();

    _precip.refr_mode();
    _pop.refr_mode();
    _wind.refr_mode();
}
