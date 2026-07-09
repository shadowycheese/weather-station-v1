#ifndef OutsideTemp_h
#define OutsideTemp_h

#include "lv.hpp"
#include "ui/core/card.h"
#include "ui/theme.h"

class OutsideTemp : public Card
{
public:
    OutsideTemp();

    virtual void init(lv::ObjectView parent);

private:
    lv::ObjectView _root;
    lv::Label _temperature;
    lv::Spangroup _minMax;
    lv::Label _rainfall;
    lv::Label _humidity;
    lv::Label _pressure;

    lv_span_t *_min, *_max;
};

#endif