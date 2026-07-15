#include "models.h"
#include "events/edt.h"
#include "net/forecast-task.h"
#include "ui/outside-temp.h"
#include "ui/theme.h"
#include "ui/ui-helpers.h"
#include "ui/fonts/fonts.h"
#include "ui/extensions/style-wrapper.hpp"

#define TEMP_WIDTH 450
#define TEMP_HEIGHT 300

OutsideTemp::OutsideTemp()
{
}

void OutsideTemp::init(lv::ObjectView parent)
{
    auto box = create_hbox(parent, SCREEN_WIDTH, TEMP_HEIGHT);

    auto lbox = create_vbox(box, TEMP_WIDTH, TEMP_HEIGHT)
                    .align(LV_ALIGN_TOP_LEFT)
                    .bg_color(COL_BACKGROUND_2)
                    .border_side((lv_border_side_t)(LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_RIGHT))
                    .padding(0)
                    .outline_width(1);

    auto tbox = create_hbox(lbox, TEMP_WIDTH + 10, 48)
                    .align(LV_ALIGN_TOP_LEFT)
                    .bg_color(COL_BACKGROUND_2)
                    .border_width(0)
                    .padding(0)
                    .outline_width(0);
    lv::Label::create(tbox)
        .text("Outside Temperature")
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT_BOLD)
        .border_color(COL_BACKGROUND_2)
        .border_width(6)
        .align(LV_ALIGN_TOP_LEFT);

    _weather = lv::Image::create(tbox)
                   .align(LV_ALIGN_TOP_RIGHT)
                   .offset_y(-36)
                   .fill_width()
                   .scale(128);

    _temperature = lv::Label::create(lbox)
                       .text_color(COL_TEXT_PRIMARY)
                       .text_font(FONT_144PT_BOLD)
                       .text_align(LV_TEXT_ALIGN_CENTER)
                       .fill_width()
                       .border_width(0)
                       .margin_top(-10)
                       .margin_bottom(-10)
                       .text("wait")
                       .align(LV_ALIGN_CENTER);

    auto lbox2 = create_hbox(lbox, 0, 60)
                     .border_color(COL_RED)
                     .bg_color(COL_BACKGROUND_2)
                     .outline_width(0)
                     .fill_width()
                     .align(LV_ALIGN_CENTER);

    _minMax = lv::Spangroup::create(lbox2)
                  .fill_width()
                  .margin_top(-7)
                  .text_align(LV_TEXT_ALIGN_CENTER)
                  .text_color(COL_TEXT_SECONDARY)
                  .text_font(FONT_24PT_BOLD);

    auto minTitle = _minMax.new_span();

    _min = _minMax.new_span();

    auto sep = _minMax.new_span();
    auto maxTitle = _minMax.new_span();

    _max = _minMax.new_span();

    _minMax.span_text(minTitle, "Min: ");
    _minMax.span_text(sep, " | ");
    _minMax.span_text(maxTitle, "Max: ");

    style_wrapper(_minMax.span_style(sep))
        .text_color(COL_TEXT_MUTED)
        .text_font(FONT_24PT);
    style_wrapper(_minMax.span_style(_min))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_36PT_BOLD);
    style_wrapper(_minMax.span_style(_max))
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_36PT_BOLD);

    lv::Flex rbox = create_vbox(box, SCREEN_WIDTH - (TEMP_WIDTH + 2), TEMP_HEIGHT)
                        .align(LV_ALIGN_TOP_LEFT)
                        .padding(0)
                        .border_width(0)
                        .outline_width(1)
                        .bg_color(COL_BACKGROUND_2);

    lv::Label::create(rbox)
        .text("Rainfall (last 24h)")
        .border_color(COL_BACKGROUND_2)
        .border_width(6)
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT_BOLD);

    _rainfall = lv::Label::create(rbox)
                    .text("0.0 mm")
                    .fill_width()
                    .pad_left(20)
                    .margin_top(-6)
                    .text_color(COL_TEXT_PRIMARY)
                    .text_font(FONT_36PT_BOLD);

    lv::Label::create(rbox)
        .text("Humidity")
        .border_color(COL_BACKGROUND_2)
        .border_width(6)
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT_BOLD);

    _humidity = lv::Label::create(rbox)
                    .text("-")
                    .fill_width()
                    .margin_top(-6)
                    .pad_left(20)
                    .text_color(COL_TEXT_PRIMARY)
                    .text_font(FONT_36PT_BOLD);

    lv::Label::create(rbox)
        .text("Pressure")
        .border_color(COL_BACKGROUND_2)
        .border_width(6)
        .text_color(COL_TEXT_SECONDARY)
        .text_font(FONT_24PT_BOLD);

    _pressure = lv::Label::create(rbox)
                    .text("-")
                    .text_color(COL_TEXT_PRIMARY)
                    .margin_top(-6)
                    .pad_left(20)
                    .text_font(FONT_36PT_BOLD)
                    .bg_color(COL_BACKGROUND_2);

    edt_add_system_event_handler(SYSTEM_EVENT_FORECAST, [this](system_event_t m)
                                 {
                                     forecast_t fc;
                                     get_forecast(&fc);
                                     _weather.src(weather_code_icon(&fc.current)); //
                                 });

    edt_add_metric_event_handler(METRIC_OUTSIDE1_BME280_TEMPERATURE, [this](metric_event_t m)
                                 {
                                     char temp[10];

                                     snprintf(temp, 10, "%0.1f", m.value);

                                     _temperature.text(temp);
                                     _temperature.invalidate();

                                     if (m.attributes & IS_24_HR_HIGH)
                                     {
                                         _minMax.span_text(_max, temp);
                                     }
                                     if (m.attributes & IS_24_HR_LOW)
                                     {
                                         _minMax.span_text(_min, temp);
                                     }
                                     _minMax.refr_mode(); //
                                 });

    edt_add_metric_event_handler(METRIC_OUTSIDE_RAINFULL_MM_24HOURS, [this](metric_event_t m)
                                 {
                                     char temp[10];

                                     snprintf(temp, 10, "%0.1f%%", m.value);

                                     _rainfall.text(temp);
                                     _rainfall.invalidate(); //
                                 });

    edt_add_metric_event_handler(METRIC_OUTSIDE1_BME280_HUMIDITY, [this](metric_event_t m)
                                 {
                                     char temp[10];

                                     snprintf(temp, 10, "%0.1f%%", m.value);

                                     _humidity.text(temp);
                                     _humidity.invalidate(); //
                                 });

    edt_add_metric_event_handler(METRIC_OUTSIDE1_BME280_PRESSURE, [this](metric_event_t m)
                                 {
                                     char temp[10];

                                     snprintf(temp, 10, "%0.1f hPa", m.value);

                                     _pressure.text(temp);
                                     _pressure.invalidate(); //
                                 });
}