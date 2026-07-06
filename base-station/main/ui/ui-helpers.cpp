#include "ui-helpers.h"
#include "theme.h"

extern "C"
{
    lv::Flex create_box(lv_obj_t *parent, int width, int height)
    {
        return lv::vbox((lv::ObjectView)parent)
            .width(width)
            .height(height)
            .bg_color(COL_BACKGROUND)
            .radius(0)
            .border_width(0)
            .align(LV_ALIGN_TOP_LEFT);
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
