#ifndef ESPADC_H
#define ESPADC_H

#include <stdint.h>
#include "esp_adc/adc_oneshot.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern int16_t read_adc(adc_channel_t channel);

#ifdef __cplusplus
}
#endif

#endif
