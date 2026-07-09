#ifndef Header_h
#define Header_h

#include "lv.hpp"
#include "ui/core/card.h"

class Header : public Card
{
public:
    Header();

    virtual void init(lv::ObjectView);

private:
    lv::ObjectView _box;
    lv::Label _labelTime;
    lv::Label _labelDate;
    lv::Label _labelWifi;
};

#endif