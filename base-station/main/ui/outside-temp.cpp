#include "models.h"
#include "events/edt.h"
#include "ui/outside-temp.h"
#include "ui/theme.h"
#include "ui/ui-helpers.h"
#include "ui/extensions/style-wrapper.hpp"

OutsideTemp::OutsideTemp()
{
}

void OutsideTemp::init(lv::ObjectView parent)
{
    lv::Flex box = create_box(parent, SCREEN_WIDTH, 280);

    _mainTemperature = lv::Spangroup::create(box)
                           .width(180)
                           .text_color(COL_TEXT_PRIMARY)
                           .bg_color(COL_BACKGROUND);

    _temperatureElement = _mainTemperature.new_span();

    lv_span_t *tempUnits = _mainTemperature.new_span();

    style_wrapper(_mainTemperature.span_style(tempUnits))
        .bg_color(COL_BACKGROUND)
        .text_font(FONT_18PT);

    _mainTemperature.span_text(tempUnits, "°C");

    style_wrapper(_mainTemperature.span_style(_temperatureElement))
        .bg_color(COL_BACKGROUND)
        .text_color(COL_TEXT_PRIMARY)
        .text_font(FONT_24PT);

    edt_add_metric_event_handler(METRIC_OUTSIDE1_BME280_TEMPERATURE, [this](metric_event_t m)
                                 {
                                     char temp[10];

                                     snprintf(temp, 10, "%0.1f", m.value);

                                     printf("RX: %s\n", temp);

                                     _mainTemperature.span_text(_temperatureElement, temp); //
                                 });
}