#ifndef Display_h
#define Display_h

#include "lv.hpp"

class Display
{
public:
    void start();

    void set_boot_status(char *status);

    void set_boot_complete();

    void setValues(char *temp, char *hum, char *pres, char *uv);

private:
    lv::Label _labelTemp;
    lv::Label _labelHumidity;
    lv::Label _labelPressure;
    lv::Label _labelUV;
    lv::Label _labelBoot;

    void create_boot_io();
};

#endif