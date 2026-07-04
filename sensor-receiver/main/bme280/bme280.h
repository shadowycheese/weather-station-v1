/**
 * BMX280 - BME280 & BMP280 Driver for Esspressif ESP-32.
 *
 * MIT License
 *
 * Copyright (C) 2020 Halit Utku Maden
 * Please contact at <utkumaden@hotmail.com>
 */

#ifndef _BMX280_H_
#define _BMX280_H_

#define CONFIG_USE_I2C_MASTER_DRIVER 1
#define CONFIG_BMX280_TIMEOUT 100
#define CONFIG_BMX280_EXPECT_BME280 1
#define CONFIG_BMX280_EXPECT_BMP280 0

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include "sdkconfig.h"

#include "bme280_bits.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

    typedef struct bmx280_t bmx280_t;

    extern bmx280_t *bmx280_create_master(i2c_master_bus_handle_t bus_handle);

    extern esp_err_t bmx280_reset(bmx280_t *bmx280);

    extern esp_err_t bmx280_device_create(bmx280_t *bmx280, const uint16_t dev_addr);

    extern void bmx280_close(bmx280_t *bmx280);

    extern esp_err_t bmx280_init(bmx280_t *bmx280);
    extern esp_err_t bmx280_configure(bmx280_t *bmx280, bmx280_config_t *cfg);

    extern esp_err_t bmx280_setMode(bmx280_t *bmx280, bmx280_mode_t mode);
    extern esp_err_t bmx280_getMode(bmx280_t *bmx280, bmx280_mode_t *mode);
    extern bool bmx280_isSampling(bmx280_t *bmx280);

    extern esp_err_t bmx280_readout(bmx280_t *bmx280, float *temperature, float *pressure, float *humidity);

#ifdef __cplusplus
};
#endif
