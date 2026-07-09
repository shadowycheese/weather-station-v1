#include "ui-helpers.h"
#include "theme.h"

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
}
