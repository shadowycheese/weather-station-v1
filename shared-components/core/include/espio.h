#ifndef ESPIO_H
#define ESPIO_H

#include <stdint.h>
#include "driver/gpio.h"

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    extern esp_err_t configure_output_pin(int pin);
    extern esp_err_t configure_input_pin(int pin, gpio_int_type_t interruptType);

#ifdef __cplusplus
}
#endif

#endif
