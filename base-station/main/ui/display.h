#ifndef Display_h
#define Display_h

#include "lv.hpp"
#include "log/log.h"
#include "ui/core/card.h"
#include "ui/header.h"
#include "ui/outside-temp.h"

class Display
{
public:
    Display();

    void start();

    static void log_message(log_level_t level, const char *message);

    void set_boot_complete();

private:
    void init_ui();

    lv::Textarea _logMessages;
    lv::ObjectView _mainPanel;

    Header _header;
    OutsideTemp _outsideTemp;

    Card *_cards[2] = {&_header, &_outsideTemp};

    static Display *_instance;
};

#endif