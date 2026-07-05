#ifndef Header_h
#define Header_h

#include "lv.hpp"
#include "ui/core/card.h"

class Header : public Card
{
public:
    Header();

    virtual int height() { return 80; }

    virtual void init(lv::ObjectView);

    virtual lv::ObjectView component() { return _box; }

private:
    lv::Flex _box;
    lv::Label _labelTime;
    lv::Label _labelDate;
    lv::Label _labelWifi;
    lv::Label _labelStatus;
};

#endif