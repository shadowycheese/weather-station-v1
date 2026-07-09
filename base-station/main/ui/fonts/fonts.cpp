#include "ui/fonts/fonts.h"
#include "resources/fonts/montserrat-bold.h"
#include "resources/fonts/montserrat-regular.h"
#include "resources/fonts/lasolid.h"

lv_font_t *ttf_font_montserrat_12;
lv_font_t *ttf_font_montserrat_14;
lv_font_t *ttf_font_montserrat_16;
lv_font_t *ttf_font_montserrat_18;
lv_font_t *ttf_font_montserrat_20;
lv_font_t *ttf_font_montserrat_22;
lv_font_t *ttf_font_montserrat_24;

lv_font_t *ttf_font_montserrat_12_bold;
lv_font_t *ttf_font_montserrat_14_bold;
lv_font_t *ttf_font_montserrat_16_bold;
lv_font_t *ttf_font_montserrat_18_bold;
lv_font_t *ttf_font_montserrat_20_bold;
lv_font_t *ttf_font_montserrat_22_bold;
lv_font_t *ttf_font_montserrat_24_bold;

lv_font_t *ttf_font_montserrat_36_bold;
lv_font_t *ttf_font_montserrat_48_bold;
lv_font_t *ttf_font_montserrat_72_bold;
lv_font_t *ttf_font_montserrat_96_bold;
lv_font_t *ttf_font_montserrat_144_bold;

lv_font_t *ttf_font_la_solid_12;
lv_font_t *ttf_font_la_solid_18;
lv_font_t *ttf_font_la_solid_24;
lv_font_t *ttf_font_la_solid_36;
lv_font_t *ttf_font_la_solid_48;
lv_font_t *ttf_font_la_solid_72;
lv_font_t *ttf_font_la_solid_96;

extern "C"
{
    void init_fonts()
    {
        const int len_reg = Montserrat_Regular_ttf_len;

        ttf_font_montserrat_12 = lv_tiny_ttf_create_data(Montserrat_Regular_ttf, len_reg, 12);
        ttf_font_montserrat_14 = lv_tiny_ttf_create_data(Montserrat_Regular_ttf, len_reg, 14);
        ttf_font_montserrat_16 = lv_tiny_ttf_create_data(Montserrat_Regular_ttf, len_reg, 16);
        ttf_font_montserrat_18 = lv_tiny_ttf_create_data(Montserrat_Regular_ttf, len_reg, 18);
        ttf_font_montserrat_20 = lv_tiny_ttf_create_data(Montserrat_Regular_ttf, len_reg, 20);
        ttf_font_montserrat_22 = lv_tiny_ttf_create_data(Montserrat_Regular_ttf, len_reg, 22);
        ttf_font_montserrat_24 = lv_tiny_ttf_create_data(Montserrat_Regular_ttf, len_reg, 24);

        const int len_bold = Montserrat_Bold_ttf_len;

        ttf_font_montserrat_12_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 12);
        ttf_font_montserrat_14_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 14);
        ttf_font_montserrat_16_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 16);
        ttf_font_montserrat_18_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 18);
        ttf_font_montserrat_20_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 20);
        ttf_font_montserrat_22_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 22);
        ttf_font_montserrat_24_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 24);

        ttf_font_montserrat_36_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 36);
        ttf_font_montserrat_48_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 48);
        ttf_font_montserrat_72_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 72);
        ttf_font_montserrat_96_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 96);
        ttf_font_montserrat_144_bold = lv_tiny_ttf_create_data(Montserrat_Bold_ttf, len_bold, 144);

        const int len_sym = la_solid_900_ttf_len;

        ttf_font_la_solid_12 = lv_tiny_ttf_create_data(la_solid_900_ttf, len_sym, 12);
        ttf_font_la_solid_24 = lv_tiny_ttf_create_data(la_solid_900_ttf, len_sym, 24);
        ttf_font_la_solid_36 = lv_tiny_ttf_create_data(la_solid_900_ttf, len_sym, 36);
        ttf_font_la_solid_48 = lv_tiny_ttf_create_data(la_solid_900_ttf, len_sym, 48);
        ttf_font_la_solid_72 = lv_tiny_ttf_create_data(la_solid_900_ttf, len_sym, 72);
        ttf_font_la_solid_96 = lv_tiny_ttf_create_data(la_solid_900_ttf, len_sym, 96);
    }
}
