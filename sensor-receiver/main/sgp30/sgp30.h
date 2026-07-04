#pragma once

#include <esp_err.h>
#include <stdint.h>
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C"
{
#endif
    esp_err_t sgp30_init(i2c_master_bus_handle_t bus_handle, uint16_t dev_addr, int speed);

    esp_err_t sgp30_read_measurements(uint16_t *co2, uint16_t *tvoc);

    esp_err_t sgp30_get_serial_id(uint8_t *serial_id);

    esp_err_t sgp30_set_humidity(float absolute_humidity_g_m3);

#ifdef __cplusplus
}
#endif
