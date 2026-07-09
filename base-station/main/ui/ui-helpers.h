#ifndef UI_Helpers_H
#define UI_Helpers_H

#include "lv.hpp"
#include "ui/extensions/style-wrapper.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

    extern lv::Flex create_vbox(lv_obj_t *parent, int width, int height);

    extern lv::Flex create_hbox(lv_obj_t *parent, int width, int height);

    extern lv::Style new_style();

    extern lv::StyleWrapper style_wrapper(lv_style_t *style);

#ifdef __cplusplus
}
#endif

#endif
