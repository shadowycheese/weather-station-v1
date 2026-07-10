#include "ui/forecast.h"
#include "ui/extensions/style-wrapper.hpp"
#include "net/forecast-task.h"

#define FORECAST_HEIGHT 60

void ForecastDay::init(lv::ObjectView parent)
{
    auto box = create_hbox(parent, SCREEN_WIDTH, FORECAST_HEIGHT);

    _forecast = lv::Spangroup::create(box)
                    .fill_width()
                    .margin_top(-7)
                    .text_align(LV_TEXT_ALIGN_CENTER)
                    .text_color(COL_TEXT_SECONDARY)
                    .text_font(FONT_24PT_BOLD);

    lv_span_t *seps[2];

    _date = _forecast.new_span();
    _temp_max = _forecast.new_span();
    _weather = _forecast.new_span();
    seps[0] = _forecast.new_span();
    _wind = _forecast.new_span();
    _wind_dir = _forecast.new_span();
    seps[1] = _forecast.new_span();
    _pop = _forecast.new_span();

    for (int i = 0; i < 2; i++)
    {
        style_wrapper(_forecast.span_style(seps[i]))
            .text_color(COL_TEXT_MUTED)
            .text_font(FONT_20PT);
    }

    style_wrapper(_forecast.span_style(_date))
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT_BOLD);
    style_wrapper(_forecast.span_style(_temp_max))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_36PT_BOLD)
        .text_align(LV_TEXT_ALIGN_CENTER)
        .min_width(250)
        .max_width(250);
    style_wrapper(_forecast.span_style(_weather))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_SYMBOL_36PT)
        .text_align(LV_TEXT_ALIGN_CENTER)
        .min_width(70)
        .max_width(70);
    style_wrapper(_forecast.span_style(_wind))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_36PT_BOLD)
        .text_align(LV_TEXT_ALIGN_CENTER)
        .min_width(150)
        .max_width(150);
    style_wrapper(_forecast.span_style(_wind_dir))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_36PT_BOLD)
        .text_align(LV_TEXT_ALIGN_CENTER)
        .min_width(70)
        .max_width(70);
    style_wrapper(_forecast.span_style(_pop))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_36PT_BOLD)
        .text_align(LV_TEXT_ALIGN_CENTER)
        .min_width(150)
        .max_width(150);

    clear();
}

void ForecastDay::clear()
{
    _forecast.span_text(_date, "-");
    _forecast.span_text(_temp_max, "-");
    _forecast.span_text(_weather, "-");
    _forecast.span_text(_wind, "-");
    _forecast.span_text(_wind_dir, "-");
    _forecast.span_text(_pop, "-");

    style_wrapper(_forecast.span_style(_date)).text_color(COL_TEXT_MUTED);
    style_wrapper(_forecast.span_style(_temp_max)).text_color(COL_TEXT_MUTED);
    style_wrapper(_forecast.span_style(_weather)).text_color(COL_TEXT_MUTED);
    style_wrapper(_forecast.span_style(_wind)).text_color(COL_TEXT_MUTED);
    style_wrapper(_forecast.span_style(_wind_dir)).text_color(COL_TEXT_MUTED);
    style_wrapper(_forecast.span_style(_pop)).text_color(COL_TEXT_MUTED);

    _forecast.refr_mode();
}

void ForecastDay::update(forecast_day_t *fc)
{
    char temp[10], wind[16], pop[10];

    snprintf(temp, 10, " %0.1f ", fc->temp_max);
    snprintf(wind, 16, " %0.1f km/h ", fc->wind_speed);
    snprintf(pop, 10, " %0.1f%% ", fc->pop);

    _forecast.span_text(_date, fc->date);
    _forecast.span_text(_temp_max, temp);
    _forecast.span_text(_weather, weather_code_icon(fc->weather_code));
    _forecast.span_text(_wind, wind);
    _forecast.span_text(_wind_dir, wind_dir(fc->wind_dir));
    _forecast.span_text(_pop, pop);

    style_wrapper(_forecast.span_style(_temp_max)).text_color(COL_TEXT_PRIMARY);
    style_wrapper(_forecast.span_style(_weather)).text_color(weather_code_color(fc->weather_code));
    style_wrapper(_forecast.span_style(_wind)).text_color(wind_speed_color(fc->wind_speed));
    style_wrapper(_forecast.span_style(_wind_dir)).text_color(COL_TEXT_SECONDARY);
    style_wrapper(_forecast.span_style(_pop)).text_color(pop_color(fc->pop));

    _forecast.refr_mode();
}

const char *ForecastDay::wind_dir(int16_t wind_dir)
{
    const char *points[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

    // 3. Offset by 22.5 to center the sectors, then divide by 45 degrees
    int index = (int)((wind_dir + 22.5) / 45.0) % 8;

    return points[index];
}

lv_color_t ForecastDay::wind_speed_color(float windspeed)
{
    if (windspeed < 15.0f)
    {
        return COL_TEXT_PRIMARY;
    }
    else if (windspeed < 25)
    {
        return COL_TEXT_WARNING;
    }
    else
    {
        return COL_TEXT_ERROR;
    }
}

lv_color_t ForecastDay::pop_color(float pop)
{
    if (pop < 15.0f)
    {
        return COL_TEXT_PRIMARY;
    }
    else if (pop < 30)
    {
        return COL_TEXT_WARNING;
    }
    else
    {
        return COL_TEXT_ERROR;
    }
}

const char *ForecastDay::weather_code_icon(int16_t wc)
{
    switch (wc)
    {
    case 0:              // Clear
        return "\uf185"; // la-sun

    case 1:              // Mainly clear
        return "\uf6c4"; // la-cloud-sun

    case 2:              // Partly cloudy
        return "\uf6c4"; // la-cloud-sun

    case 3:              // Overcast
        return "\uf0c2"; // la-cloud

    case 45:
    case 48:             // Fog
        return "\uf74e"; // la-smog

    case 51:
    case 53:
    case 55:
    case 56:
    case 57:             // Drizzle
        return "\uf73d"; // la-cloud-rain

    case 61:
    case 63:
    case 65:
    case 66:
    case 67:             // Rain
        return "\uf740"; // la-cloud-showers-heavy

    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:             // Snow
        return "\uf2dc"; // la-snowflake

    case 80:
    case 81:
    case 82:             // Rain showers
        return "\uf740"; // la-cloud-showers-heavy

    case 95:
    case 96:
    case 99:             // Thunderstorm
        return "\uf76c"; // la-bolt

    default:
        return "\uf128"; // la-question
    }
}

lv_color_t ForecastDay::weather_code_color(int16_t wc)
{
    switch (wc)
    {
    case 0:                      // Clear
        return COL_TEXT_PRIMARY; // la-sun

    case 1:                      // Mainly clear
        return COL_TEXT_PRIMARY; // la-cloud-sun

    case 2:                        // Partly cloudy
        return COL_TEXT_SECONDARY; // la-cloud-sun

    case 3:                        // Overcast
        return COL_TEXT_SECONDARY; // la-cloud

    case 45:
    case 48:                       // Fog
        return COL_TEXT_SECONDARY; // la-smog

    case 51:
    case 53:
    case 55:
    case 56:
    case 57:                       // Drizzle
        return COL_TEXT_SECONDARY; // la-cloud-rain

    case 61:
    case 63:
    case 65:
    case 66:
    case 67:                       // Rain
        return COL_TEXT_SECONDARY; // la-cloud-showers-heavy

    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:             // Snow
        return COL_CYAN; // la-snowflake

    case 80:
    case 81:
    case 82:                       // Rain showers
        return COL_TEXT_SECONDARY; // la-cloud-showers-heavy

    case 95:
    case 96:
    case 99:                       // Thunderstorm
        return COL_TEXT_SECONDARY; // la-bolt

    default:
        return COL_TEXT_MUTED; // la-question
    }
}
