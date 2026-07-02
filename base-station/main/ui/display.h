#ifndef Display_h
#define Display_h

#include "lv.hpp"
#include "log/log.h"

class Display
{
public:
    Display();

    void start();

    static void log_message(log_level_t level, const char *message);

    void set_boot_complete();

    void setValues(char *temp, char *hum, char *pres, char *uv);

    static void boot_message(uint8_t level, const char *message);

private:
    lv::Label _labelTemp;
    lv::Label _labelHumidity;
    lv::Label _labelPressure;
    lv::Label _labelUV;
    lv::Textarea _logMessages;

    void init_ui();

    static Display *_instance;
};

#endif