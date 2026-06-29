#ifndef Display_h
#define Display_h

#include "lv.hpp"

class Display
{
public:
    void start();

    void setText(char *text);

private:
    lv::Label _label;

    void create_cpp20_ui();
};

#endif