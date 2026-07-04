#include "bme280.h"
#include "esp_log.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BME280_REG_HUMI_LSB 0xFE
#define BME280_REG_HUMI_MSB 0xFD
#define BME280_REG_TEMP_XSB 0xFC
#define BME280_REG_TEMP_LSB 0xFB
#define BME280_REG_TEMP_MSB 0xFA
#define BME280_REG_PRES_XSB 0xF9
#define BME280_REG_PRES_LSB 0xF8
#define BME280_REG_PRES_MSB 0xF7
#define BME280_REG_CONFIG 0xF5
#define BME280_REG_MESCTL 0xF4
#define BME280_REG_STATUS 0xF3
#define BME280_REG_HUMCTL 0xF2
#define BME280_REG_CAL_HI 0xE1
#define BME280_REG_CAL_LO 0x88
#define BME280_REG_RESET 0xE0
#define BME280_RESET_VEC 0xB6
#define BME280_REG_CHPID 0xD0
#define BME280_ID 0x60
#define BMP280_ID0 0x56
#define BMP280_ID1 0x57
#define BMP280_ID2 0x58

#define BME280_TIMEOUT 100

typedef struct
{
    struct
    {
        uint16_t T1;
        int16_t T2;
        int16_t T3;
        uint16_t P1;
        int16_t P2;
        int16_t P3;
        int16_t P4;
        int16_t P5;
        int16_t P6;
        int16_t P7;
        int16_t P8;
        int16_t P9;
        uint8_t H1;
        int16_t H2;
        uint8_t H3;
        int16_t H4;
        int16_t H5;
        int8_t H6;
    } cmps;
    int32_t t_fine;
} bme280_t;

static bme280_t _bme280 = {};
static i2c_master_dev_handle_t _bme280_i2c_dev_handle = NULL;

static esp_err_t bme280_read(uint8_t addr, uint8_t *dout, size_t size)
{
    return i2c_master_transmit_receive(_bme280_i2c_dev_handle, &addr, sizeof(addr), dout, size, BME280_TIMEOUT);
}

static esp_err_t bme280_write(uint8_t addr, const uint8_t *din, size_t size)
{
    esp_err_t err;
    for (uint8_t i = 0; i < size; i++)
    {
        uint8_t dat[2] = {(addr + i), din[i]};
        if ((err = i2c_master_transmit(_bme280_i2c_dev_handle, dat, 2, BME280_TIMEOUT)) != ESP_OK)
        {
            return err;
        }
    }
    return ESP_OK;
}

esp_err_t bme280_set_measurement_mode()
{
    // Sets Humidity to ×4 oversampling
    uint8_t ctrl_mes = 0x03;
    esp_err_t err = bme280_write(BME280_REG_HUMCTL, &ctrl_mes, 1);

    if (err != ESP_OK)
    {
        return err;
    }

    //  Sets Temp to ×2, Pressure to ×8, and triggers Forced Mode).
    ctrl_mes = 0x6F;
    err = bme280_write(BME280_REG_MESCTL, &ctrl_mes, 1);

    return err;
}

static esp_err_t bme280_calibrate()
{
    ESP_LOGI("bme280", "Reading out calibration values...");

    esp_err_t err;
    uint8_t buf[26];

    err = bme280_read(BME280_REG_CAL_LO, buf, sizeof buf);

    if (err != ESP_OK)
    {
        return err;
    }

    ESP_LOGI("bme280", "Read Low Bank.");

    _bme280.cmps.T1 = buf[0] | (buf[1] << 8);
    _bme280.cmps.T2 = buf[2] | (buf[3] << 8);
    _bme280.cmps.T3 = buf[4] | (buf[5] << 8);
    _bme280.cmps.P1 = buf[6] | (buf[7] << 8);
    _bme280.cmps.P2 = buf[8] | (buf[9] << 8);
    _bme280.cmps.P3 = buf[10] | (buf[11] << 8);
    _bme280.cmps.P4 = buf[12] | (buf[13] << 8);
    _bme280.cmps.P5 = buf[14] | (buf[15] << 8);
    _bme280.cmps.P6 = buf[16] | (buf[17] << 8);
    _bme280.cmps.P7 = buf[18] | (buf[19] << 8);
    _bme280.cmps.P8 = buf[20] | (buf[21] << 8);
    _bme280.cmps.P9 = buf[22] | (buf[23] << 8);

    // First get H1 out of the way.
    _bme280.cmps.H1 = buf[23];

    err = bme280_read(BME280_REG_CAL_HI, buf, 7);

    if (err != ESP_OK)
        return err;

    ESP_LOGI("bme280", "Read High Bank.");

    _bme280.cmps.H2 = buf[0] | (buf[1] << 8);
    _bme280.cmps.H3 = buf[2];
    _bme280.cmps.H4 = (buf[3] << 4) | (buf[4] & 0x0F);
    _bme280.cmps.H5 = (buf[4] >> 4) | (buf[5] << 4);
    _bme280.cmps.H6 = buf[6];

    return ESP_OK;
}

esp_err_t bme280_reset()
{
    const static uint8_t din[] = {BME280_RESET_VEC};
    return bme280_write(BME280_REG_RESET, din, sizeof din);
}

esp_err_t bme280_reset_and_configure()
{
    esp_err_t error = bme280_reset();

    if (error == ESP_OK)
    {
        // Give the sensor 10 ms delay to reset.
        vTaskDelay(pdMS_TO_TICKS(10));

        // Read calibration data.
        bme280_calibrate();

        // Apply settings
        bme280_set_measurement_mode();

        ESP_LOGI("bme280", "Dumping calibration...");
        ESP_LOG_BUFFER_HEX("bme280", &_bme280.cmps, sizeof(_bme280.cmps));
    }
    else
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(error);
    }

    return error;
}

esp_err_t bme280_init(i2c_master_bus_handle_t bus_handle, const uint16_t dev_addr, const int speed)
{
    i2c_device_config_t dev_config = {};
    dev_config.device_address = dev_addr;
    dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_config.scl_speed_hz = speed;

    ESP_LOGI("bme280", "device_create for BMP280/BME280 sensors on ADDR %X @ %dHz", dev_addr, speed);

    esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_config, &_bme280_i2c_dev_handle);
    if (err == ESP_OK)
    {
        ESP_LOGI("bme280", "device_create success on 0x%x", dev_addr);
        return bme280_reset_and_configure(_bme280);
    }
    else
    {
        ESP_LOGE("bme280", "device_create error on 0x%x", dev_addr);
        return err;
    }
}

float bme280_compenstate_temperature(uint8_t *data)
{
    int32_t var1, var2;
    int32_t adc_T = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | data[2];
    if (adc_T == 0x800000)
    {
        return ESP_FAIL;
    }
    adc_T >>= 4;

    var1 = ((((adc_T >> 3) - ((int32_t)_bme280.cmps.T1 << 1))) * ((int32_t)_bme280.cmps.T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)_bme280.cmps.T1)) * ((adc_T >> 4) - ((int32_t)_bme280.cmps.T1))) >> 12) * ((int32_t)_bme280.cmps.T3)) >> 14;

    _bme280.t_fine = var1 + var2;
    return ((_bme280.t_fine * 5 + 128) >> 8) / 100.0;
}

float bme280_compenstate_pressure(uint8_t *data)
{
    int32_t adc_P = (data[0] << 16) | (data[1] << 8) | data[2];
    if (adc_P == 0x800000)
    {
        return ESP_FAIL;
    }
    int64_t var1, var2, p;
    adc_P >>= 4;
    var1 = ((int64_t)_bme280.t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)_bme280.cmps.P6;
    var2 = var2 + ((var1 * (int64_t)_bme280.cmps.P5) << 17);
    var2 = var2 + (((int64_t)_bme280.cmps.P4) << 35);
    var1 = ((var1 * var1 * (int64_t)_bme280.cmps.P3) >> 8) + ((var1 * (int64_t)_bme280.cmps.P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)_bme280.cmps.P1) >> 33;
    if (var1 == 0)
    {
        return ESP_FAIL;
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)_bme280.cmps.P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)_bme280.cmps.P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)_bme280.cmps.P7) << 4);
    p = p >> 8; // /256
    return (float)p / 100;
}

float bme280_compenstate_humidity(uint8_t *data)
{
    uint16_t data16 = ((uint16_t)data[0] << 8) | data[1];
    int32_t adc_H = data16;

    if (adc_H == 0x8000)
    {
        return ESP_FAIL;
    }

    int32_t v_x1_u32r;
    v_x1_u32r = (_bme280.t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)_bme280.cmps.H4) << 20) - (((int32_t)_bme280.cmps.H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)_bme280.cmps.H6)) >> 10) * (((v_x1_u32r * ((int32_t)_bme280.cmps.H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)_bme280.cmps.H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)_bme280.cmps.H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    return (v_x1_u32r >> 12) / 1024.0;
}

esp_err_t bme280_readout(float *temperature, float *pressure, float *humidity)
{
    uint8_t buffer[3];
    esp_err_t error;

    if ((error = bme280_read(BME280_REG_TEMP_MSB, buffer, 3)) != ESP_OK)
    {
        return error;
    }

    *temperature = bme280_compenstate_temperature(buffer);

    if ((error = bme280_read(BME280_REG_PRES_MSB, buffer, 3)) != ESP_OK)
    {
        return error;
    }

    *pressure = bme280_compenstate_pressure(buffer);

    if ((error = bme280_read(BME280_REG_HUMI_MSB, buffer, 2)) != ESP_OK)
    {
        return error;
    }

    *humidity = bme280_compenstate_humidity(buffer);

    if (*pressure < 0 || *humidity < 0)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}