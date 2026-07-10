#include "ui/forecast.h"
#include "ui/extensions/style-wrapper.hpp"
#include "ui/icons/icons.h"
#include "net/forecast-task.h"

#define FORECAST_HEIGHT 60

void ForecastDay::init(lv::ObjectView parent)
{
    auto box = create_hbox(parent, SCREEN_WIDTH, FORECAST_HEIGHT);

    _day = lv::Label::create(box)
               .margin_top(-7)
               .text_align(LV_TEXT_ALIGN_CENTER)
               .text_color(COL_TEXT_SECONDARY)
               .text_font(FONT_24PT_BOLD);

    _weather = lv::Image::create(box);

    _temp = lv::Label::create(box)
                .margin_top(-7)
                .text_align(LV_TEXT_ALIGN_CENTER)
                .text_color(COL_TEXT_PRIMARY)
                .text_font(FONT_36PT_BOLD);

    _pop = lv::Spangroup::create(box)
               .fill_width()
               .margin_top(-7)
               .text_align(LV_TEXT_ALIGN_CENTER)
               .text_color(COL_TEXT_SECONDARY)
               .text_font(FONT_24PT);

    auto popTitle = _pop.new_span();
    _pop_pc = _pop.new_span();
    auto popUnits = _pop.new_span();

    style_wrapper(_precip.span_style(popTitle))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT);
    style_wrapper(_precip.span_style(popUnits))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT);
    style_wrapper(_precip.span_style(_pop_pc))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_24PT_BOLD);

    _pop.span_text(popTitle, "POP");
    _pop.span_text(popUnits, "%");
    _pop.span_text(_pop_pc, "-");

    _precip = lv::Spangroup::create(box)
                  .fill_width()
                  .margin_top(-7)
                  .text_align(LV_TEXT_ALIGN_CENTER)
                  .text_color(COL_TEXT_SECONDARY)
                  .text_font(FONT_24PT);

    auto pr_title = _precip.new_span();
    _pr_total = _precip.new_span();
    _pr_units = _precip.new_span();

    _precip.span_text(pr_title, "Total");
    _precip.span_text(_pr_total, "-");
    _precip.span_text(_pr_units, " mm");

    style_wrapper(_precip.span_style(pr_title))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT);
    style_wrapper(_precip.span_style(_pr_units))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT);
    style_wrapper(_wind.span_style(_pr_total))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_24PT_BOLD);

    lv::Image::create(box).src(icon_wind).recolor(COL_TEXT_SECONDARY);

    _wind = lv::Spangroup::create(box)
                .fill_width()
                .margin_top(-7)
                .text_align(LV_TEXT_ALIGN_CENTER)
                .text_color(COL_TEXT_SECONDARY)
                .text_font(FONT_24PT);

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
        .text_font(FONT_24PT_BOLD);
    style_wrapper(_wind.span_style(_wind_gust))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_24PT_BOLD);

    _wind.span_text(wind_sep, " | ");
    _wind.span_text(wind_units, " km/h");
    _wind.span_text(_wind_speed, "-");
    _wind.span_text(_wind_gust, "-");

    _wind_dir = lv::Image::create(box).src(icon_arrow_up);

    clear();
}

void ForecastDay::clear()
{
    _day.text("-");
    _pop.span_text(_pop_pc, "-");
    _precip.span_text(_pr_total, "-");
    _wind.span_text(_wind_speed, "-");
    _wind.span_text(_wind_gust, "-");

    _precip.refr_mode();
    _pop.refr_mode();
    _wind.refr_mode();
    _day.invalidate();
}

void ForecastDay::update(forecast_day_t *fc)
{
    char temp[10], wind[10], gust[10], pop[10], precip[10];

    snprintf(temp, 10, "%0.1f", fc->temp_max);
    snprintf(wind, 10, "%0.1f", fc->wind_speed);
    snprintf(gust, 10, "%0.1f", fc->wind_gust);
    snprintf(pop, 10, " %d", fc->pop);

    if (fc->snow > fc->rain)
    {
        snprintf(precip, 10, " %0.1f", fc->snow + (fc->rain / 10));

        _precip.span_text(_pr_units, "cm");
        _precip.span_text(_pr_total, precip);
    }
    else
    {
        snprintf(precip, 10, " %0.1f", fc->rain + (fc->snow * 10));

        _precip.span_text(_pr_units, "mm");
        _precip.span_text(_pr_total, precip);
    }

    _weather.src(weather_code_icon(fc));
    _wind_dir.rotation(fc->wind_dir * 10);

    _precip.refr_mode();
    _pop.refr_mode();
    _wind.refr_mode();
    _day.invalidate();
}

const uint8_t *ForecastDay::weather_code_icon(forecast_day_t *fc)
{
    if (fc->rain > 0.0 && fc->snow > 0.0)
    {
        return icon_rainy_7;
    }

    // 2. Standard WMO and Gap Code Evaluation
    switch (fc->weather_code)
    {
    // Clear
    case 0:
        return icon_day;

    // Clouds / Overcast
    case 1:
        return icon_cloudy_day_1;
    case 2:
        return icon_cloudy_day_3;
    case 3:
        return icon_cloudy;

    // Fog & Rime
    case 45:
    case 48:
        return icon_fog;

    // Drizzle (Standard & Intermittent Gaps)
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
        return icon_rainy_1;

    // Freezing Drizzle / Freezing Rain
    case 56:
    case 57:
    case 66:
    case 67:
        return icon_rainy_7;

    // Rain (Standard & Intermittent Gaps)
    case 61:
        return icon_rainy_4;
    case 62:
    case 63:
        return icon_rainy_5;
    case 64:
    case 65:
        return icon_rainy_6;

    // Snow & Grains (Standard & Intermittent Gaps)
    case 71:
    case 72:
        return icon_snowy_4;
    case 73:
    case 74:
        return icon_snowy_5;
    case 75:
        return icon_snowy_6;
    case 77:
        return icon_snowy_4;

    // Rain Showers
    case 80:
        return icon_rainy_4;
    case 81:
        return icon_rainy_5;
    case 82:
        return icon_rainy_6;

    // Snow Showers
    case 85:
        return icon_snowy_4;
    case 86:
        return icon_snowy_5;

    // Thunderstorms
    case 95:
        return icon_thunder;
    case 96:
    case 99:
        return icon_thunder;

    // Catch-all safety fallback
    default:
        return icon_cloudy;
    }
}
