#ifndef MyDisplay_h
#define MyDisplay_h

#include "lv.hpp"

class MyDisplay
{
    public:
        void start();

        void setText(char *text)
        {
            _label.text(text);
        }

    private:
        lv::Label _label;

        void create_cpp20_ui();
};

#endif