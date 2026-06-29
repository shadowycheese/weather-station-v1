#ifndef ESPI2C_H
#define ESPI2C_H

#include <stdint.h>
#include "driver/i2c.h"

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    extern i2c_config_t create_i2c_input_configration(int sdaPin, int sclPin, int freq);

#ifdef __cplusplus
}
#endif

#endif
