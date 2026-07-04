#ifndef _BME280_H_
#define _BME280_H_

#include <stdint.h>

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern esp_err_t bme280_reset();

    extern esp_err_t bme280_init(i2c_master_bus_handle_t bus_handle, const uint16_t dev_addr, const int speed, bool force_mode);

    extern esp_err_t bme280_delete();

    extern esp_err_t bme280_readout(float *temperature, float *pressure, float *humidity);

#ifdef __cplusplus
};
#endif
#endif
