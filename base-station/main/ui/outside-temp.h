#ifndef OutsideTemp_h
#define OutsideTemp_h

#include "lv.hpp"
#include "ui/core/card.h"

class OutsideTemp : public Card
{
public:
    OutsideTemp();

    virtual int height() { return 280; }

    virtual void init(lv::ObjectView parent);

    virtual lv::ObjectView component() { return _root; }

private:
    lv::ObjectView _root;
    lv::Spangroup _sgTemp;
    lv::Spangroup _sgMinMax;
    lv::Spangroup _sgHumidity;
    lv::Spangroup _sgPressire;
};

#endif