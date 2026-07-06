#ifndef Theme_h
#define Theme_h

#include "lv.hpp"
#include "ui/fonts/fonts.h"

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 1280

#define FONT_72PT &lv_font_montserrat_72
#define FONT_48PT &lv_font_montserrat_48
#define FONT_36PT &lv_font_montserrat_36
#define FONT_24PT &lv_font_montserrat_24
#define FONT_18PT &lv_font_montserrat_18
#define FONT_12PT &lv_font_montserrat_12

#ifdef __cplusplus
extern "C"
{
#endif

    extern const lv_color_t COL_BACKGROUND;
    extern const lv_color_t COL_TEXT_PRIMARY;
    extern const lv_color_t COL_TEXT_SECONDARY;
    extern const lv_color_t COL_TEXT_INFO;
    extern const lv_color_t COL_TEXT_WARNING;
    extern const lv_color_t COL_TEXT_ERROR;

#ifdef __cplusplus
}
#endif

#endif
