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
    lv::Spangroup _mainTemperature;
    lv_span_t *_temperatureElement;
    lv_span_t *_tempUnit;
    lv::Spangroup _sgMinMax;
    lv::Spangroup _sgHumidity;
    lv::Spangroup _sgPressire;
};

#endif