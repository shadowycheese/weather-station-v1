#ifndef Theme_h
#define Theme_h

#include "lv.hpp"
#include "ui/fonts/fonts.h"

#define TIME_ZONE_OFFSET (-(7L * 3600L))

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 1280

#define FONT_12PT ttf_font_montserrat_12
#define FONT_14PT ttf_font_montserrat_14
#define FONT_16PT ttf_font_montserrat_16
#define FONT_18PT ttf_font_montserrat_18
#define FONT_20PT ttf_font_montserrat_20
#define FONT_22PT ttf_font_montserrat_20
#define FONT_24PT ttf_font_montserrat_24

#define FONT_12PT_BOLD ttf_font_montserrat_12_bold
#define FONT_14PT_BOLD ttf_font_montserrat_14_bold
#define FONT_16PT_BOLD ttf_font_montserrat_16_bold
#define FONT_18PT_BOLD ttf_font_montserrat_18_bold
#define FONT_20PT_BOLD ttf_font_montserrat_20_bold
#define FONT_22PT_BOLD ttf_font_montserrat_20_bold
#define FONT_24PT_BOLD ttf_font_montserrat_24_bold
#define FONT_30PT_BOLD ttf_font_montserrat_30_bold
#define FONT_36PT_BOLD ttf_font_montserrat_36_bold
#define FONT_48PT_BOLD ttf_font_montserrat_48_bold
#define FONT_72PT_BOLD ttf_font_montserrat_72_bold
#define FONT_96PT_BOLD ttf_font_montserrat_96_bold
#define FONT_144PT_BOLD ttf_font_montserrat_144_bold

#define FONT_SYMBOL_12PT ttf_font_la_solid_12
#define FONT_SYMBOL_24PT ttf_font_la_solid_24
#define FONT_SYMBOL_36PT ttf_font_la_solid_36
#define FONT_SYMBOL_48PT ttf_font_la_solid_48
#define FONT_SYMBOL_72PT ttf_font_la_solid_72
#define FONT_SYMBOL_96PT ttf_font_la_solid_96

#ifdef __cplusplus
extern "C"
{
#endif

    extern const lv_color_t COL_BACKGROUND;
    extern const lv_color_t COL_BACKGROUND_2;
    extern const lv_color_t COL_TEXT_PRIMARY;
    extern const lv_color_t COL_TEXT_SECONDARY;
    extern const lv_color_t COL_TEXT_INFO;
    extern const lv_color_t COL_TEXT_WARNING;
    extern const lv_color_t COL_TEXT_ERROR;
    extern const lv_color_t COL_TEXT_MUTED;

    extern const lv_color_t COL_RED;
    extern const lv_color_t COL_GREEN;
    extern const lv_color_t COL_BLUE;
    extern const lv_color_t COL_CYAN;
    extern const lv_color_t COL_YELLOW;
    extern const lv_color_t COL_MAGENTA;

#ifdef __cplusplus
}
#endif

#endif
