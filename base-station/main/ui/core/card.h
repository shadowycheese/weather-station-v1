#ifndef Card_h
#define Card_h

#include "lv.hpp"

class Card
{
public:
    virtual void init(lv::ObjectView parent) = 0;
};

#endif