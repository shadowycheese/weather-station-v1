#include "ui-helpers.h"
#include "theme.h"
#include "net/forecast-task.h"
#include "ui/icons/icons.h"

extern "C"
{
    lv::Flex create_vbox(lv_obj_t *parent, int width, int height)
    {
        auto box = lv::vbox((lv::ObjectView)parent)
                       .bg_color(COL_BACKGROUND)
                       .border_color(COL_BACKGROUND)
                       .outline_width(0)
                       .border_width(0)
                       .radius(0)
                       .padding(0)
                       .align(LV_ALIGN_TOP_LEFT);

        if (width > 0)
        {
            box.width(width);
        }
        if (height > 0)
        {
            box.height(height);
        }

        return box;
    }

    lv::Flex create_hbox(lv_obj_t *parent, int width, int height)
    {
        auto box = lv::hbox((lv::ObjectView)parent)
                       .bg_color(COL_BACKGROUND)
                       .border_color(COL_BACKGROUND)
                       .radius(0)
                       .outline_width(0)
                       .border_width(0)
                       .padding(0)
                       .align(LV_ALIGN_TOP_LEFT);

        if (width > 0)
        {
            box.width(width);
        }
        if (height > 0)
        {
            box.height(height);
        }

        return box;
    }

    lv::StyleWrapper style_wrapper(lv_style_t *style)
    {
        return (lv::StyleWrapper)style;
    }

    lv::Style new_style()
    {
        lv::Style s;

        return s;
    }

    const lv_img_dsc_t *weather_code_icon(forecast_day_t *fc)
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
}
