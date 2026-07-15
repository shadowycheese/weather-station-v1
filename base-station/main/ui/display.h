#ifndef Display_h
#define Display_h

#include "lv.hpp"
#include "log/log.h"
#include "ui/core/card.h"
#include "ui/header.h"
#include "ui/outside-temp.h"
#include "ui/forecast.h"

class Display
{
public:
    void start();

    static void log_message(log_level_t level, const char *message);

    void set_boot_complete();

private:
    void init_ui();

    lv::Textarea _logMessages;
    lv::ObjectView _mainPanel;

    Header _header;
    OutsideTemp _outsideTemp;
    Forecast _forecast;

    Card *_cards[3] = {&_header, &_outsideTemp, &_forecast};

    static Display *_instance;
};

#endif