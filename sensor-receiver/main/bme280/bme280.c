#include "bme280.h"
#include "esp_log.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BMX280_REG_HUMI_LSB 0xFE
#define BMX280_REG_HUMI_MSB 0xFD
#define BMX280_REG_TEMP_XSB 0xFC
#define BMX280_REG_TEMP_LSB 0xFB
#define BMX280_REG_TEMP_MSB 0xFA
#define BMX280_REG_PRES_XSB 0xF9
#define BMX280_REG_PRES_LSB 0xF8
#define BMX280_REG_PRES_MSB 0xF7
#define BMX280_REG_CONFIG 0xF5
#define BMX280_REG_MESCTL 0xF4
#define BMX280_REG_STATUS 0xF3
#define BMX280_REG_HUMCTL 0xF2
#define BMX280_REG_CAL_HI 0xE1
#define BMX280_REG_CAL_LO 0x88
#define BMX280_REG_RESET 0xE0
#define BMX280_RESET_VEC 0xB6
#define BMX280_REG_CHPID 0xD0
#define BME280_ID 0x60
#define BMP280_ID0 0x56
#define BMP280_ID1 0x57
#define BMP280_ID2 0x58

struct bmx280_t
{
    i2c_master_dev_handle_t i2c_dev;
    i2c_device_config_t dev_cfg;
    i2c_master_bus_handle_t bus_handle;
    uint8_t chip_id;
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
    // Storage for a variable proportional to temperature.
    int32_t t_fine;
};

/**
 * Macro that identifies a chip id as BME280 or BMP280
 * @note Only use when the chip is verified to be either a BME280 or BMP280.
 * @see bmx280_verify
 * @param chip_id The chip id.
 */
#define bmx280_isBME(chip_id) ((chip_id) == BME280_ID)
/**
 * Macro to verify a the chip id matches with the expected values.
 * @note Use when the chip needs to be verified as a BME280 or BME280.
 * @see bmx280_isBME
 * @param chip_id The chip id.
 */
#define bmx280_verify(chip_id) (((chip_id) == BME280_ID) || ((chip_id) == BMP280_ID2) || ((chip_id) == BMP280_ID1) || ((chip_id) == BMP280_ID0))

/**
 * Returns false if the sensor was not found.
 * @param bmx280 The driver structure.
 */
#define bmx280_validate(bmx280) (!(bmx280->i2c_dev == NULL && bmx280->chip_id == 0xAD))

/**
 * Read from sensor.
 * @param bmx280 Driver Sturcture.
 * @param dev_addr Chip addresses.
 */
esp_err_t bmx280_device_create(bmx280_t *bmx280, const uint16_t dev_addr)
{
    ESP_LOGI("bmx280", "device_create for BMP280/BME280 sensors on ADDR %X", dev_addr);
    bmx280->dev_cfg.device_address = dev_addr;
    // Add device to the I2C bus
    esp_err_t err = i2c_master_bus_add_device(bmx280->bus_handle, &bmx280->dev_cfg, &bmx280->i2c_dev);
    if (err == ESP_OK)
    {
        ESP_LOGI("bmx280", "device_create success on 0x%x", dev_addr);
        return err;
    }
    else
    {
        ESP_LOGE("bmx280", "device_create error on 0x%x", dev_addr);
        return err;
    }
}

static esp_err_t bmx280_read(bmx280_t *bmx280, uint8_t addr, uint8_t *dout, size_t size)
{
    return i2c_master_transmit_receive(bmx280->i2c_dev, &addr, sizeof(addr), dout, size, CONFIG_BMX280_TIMEOUT);
}

static esp_err_t bmx280_write(bmx280_t *bmx280, uint8_t addr, const uint8_t *din, size_t size)
{
    esp_err_t err;
    for (uint8_t i = 0; i < size; i++)
    {
        uint8_t dat[2] = {(addr + i), din[i]};
        if ((err = i2c_master_transmit(bmx280->i2c_dev, dat, 2, CONFIG_BMX280_TIMEOUT)) != ESP_OK)
            return err;
    }
    return ESP_OK;
}

static esp_err_t bmx280_probe_address(bmx280_t *bmx280)
{
    esp_err_t err = bmx280_read(bmx280, BMX280_REG_CHPID, &bmx280->chip_id, sizeof bmx280->chip_id);
    if (err == ESP_OK)
    {
        if (bmx280->chip_id == BME280_ID)
        {
            ESP_LOGI("bmx280", "Probe success: address=%hhx, id=%hhx", bmx280->dev_cfg.device_address, bmx280->chip_id);
            return ESP_OK;
        }
        else
        {
            ESP_LOGE("bmx280", "Sensor model may be incorrect. Please check the sensor model configuration. If unsure, set it to AUTO.");
            err = ESP_ERR_NOT_FOUND;
        }
    }
    ESP_LOGW("bmx280", "Probe failure: address=%hhx, id=%hhx, reason=%s", bmx280->dev_cfg.device_address, bmx280->chip_id, esp_err_to_name(err));
    return err;
}

static esp_err_t bmx280_probe(bmx280_t *bmx280)
{
    ESP_LOGI("bmx280", "Probing for BMP280/BME280 sensors on I2C");
    esp_err_t err;
#if CONFIG_BMX280_ADDRESS_HI
    err = bmx280_device_create(bmx280, 0x77);
    if (err != ESP_OK)
        return err;
    err = bmx280_probe_address(bmx280);
    if (err != ESP_OK)
        ESP_LOGE("bmx280", "Sensor not found at 0x77 , Please check the address.");
    return err;
#elif CONFIG_BMX280_ADDRESS_LO
    err = bmx280_device_create(bmx280, 0x76);
    if (err != ESP_OK)
        return err;
    err = bmx280_probe_address(bmx280);
    if (err != ESP_OK)
        ESP_LOGE("bmx280", "Sensor not found at 0x76 , Please check the address.");
    return err;
#else
    err = bmx280_device_create(bmx280, 0x76);
    if (err != ESP_OK)
        return err;
    if ((err = bmx280_probe_address(bmx280)) != ESP_OK)
    {
        err = bmx280_device_create(bmx280, 0x77);
        if (err != ESP_OK)
            return err;
        if ((err = bmx280_probe_address(bmx280)) != ESP_OK)
        {
            ESP_LOGE("bmx280", "Sensor not found.");
            bmx280->i2c_dev = NULL;
            bmx280->chip_id = 0xAD;
        }
    }
    return err;
#endif
}

static esp_err_t bmx280_calibrate(bmx280_t *bmx280)
{
    // Honestly, the best course of action is to read the high and low banks
    // into a buffer, then put them in the calibration values. Makes code
    // endian agnostic, and overcomes struct packing issues.
    // Also the BME280 high bank is weird.
    //
    // Write and pray to optimizations is my new motto.

    ESP_LOGI("bmx280", "Reading out calibration values...");

    esp_err_t err;
    uint8_t buf[26];

    // Low Bank
    err = bmx280_read(bmx280, BMX280_REG_CAL_LO, buf, sizeof buf);

    if (err != ESP_OK)
        return err;

    ESP_LOGI("bmx280", "Read Low Bank.");

    bmx280->cmps.T1 = buf[0] | (buf[1] << 8);
    bmx280->cmps.T2 = buf[2] | (buf[3] << 8);
    bmx280->cmps.T3 = buf[4] | (buf[5] << 8);
    bmx280->cmps.P1 = buf[6] | (buf[7] << 8);
    bmx280->cmps.P2 = buf[8] | (buf[9] << 8);
    bmx280->cmps.P3 = buf[10] | (buf[11] << 8);
    bmx280->cmps.P4 = buf[12] | (buf[13] << 8);
    bmx280->cmps.P5 = buf[14] | (buf[15] << 8);
    bmx280->cmps.P6 = buf[16] | (buf[17] << 8);
    bmx280->cmps.P7 = buf[18] | (buf[19] << 8);
    bmx280->cmps.P8 = buf[20] | (buf[21] << 8);
    bmx280->cmps.P9 = buf[22] | (buf[23] << 8);

    {
        // First get H1 out of the way.
        bmx280->cmps.H1 = buf[23];

        err = bmx280_read(bmx280, BMX280_REG_CAL_HI, buf, 7);

        if (err != ESP_OK)
            return err;

        ESP_LOGI("bmx280", "Read High Bank.");

        bmx280->cmps.H2 = buf[0] | (buf[1] << 8);
        bmx280->cmps.H3 = buf[2];
        bmx280->cmps.H4 = (buf[3] << 4) | (buf[4] & 0x0F);
        bmx280->cmps.H5 = (buf[4] >> 4) | (buf[5] << 4);
        bmx280->cmps.H6 = buf[6];
    }

    return ESP_OK;
}

bmx280_t *bmx280_create_master(i2c_master_bus_handle_t bus_handle)
{
    bmx280_t *bmx280 = malloc(sizeof(bmx280_t));
    if (bmx280)
    {
        memset(bmx280, 0, sizeof(bmx280_t));
        bmx280->bus_handle = bus_handle;
        bmx280->dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        bmx280->dev_cfg.device_address = 0xDE;
        bmx280->dev_cfg.scl_speed_hz = 10000;
        bmx280->i2c_dev = NULL;
        bmx280->chip_id = 0xAD;
    }
    else
    {
        ESP_LOGE("bmx280", "Failed to allocate memory for bmx280.");
        bmx280_close(bmx280);
        return NULL;
    }
    return bmx280;
}

esp_err_t bmx280_reset(bmx280_t *bmx280)
{
    const static uint8_t din[] = {BMX280_RESET_VEC};
    return bmx280_write(bmx280, BMX280_REG_RESET, din, sizeof din);
}

void bmx280_close(bmx280_t *bmx280)
{
    if (bmx280 != NULL && bmx280->i2c_dev != NULL)
        i2c_master_bus_rm_device(bmx280->i2c_dev);
    free(bmx280);
}

esp_err_t bmx280_init(bmx280_t *bmx280)
{
    if (bmx280 == NULL)
        return ESP_ERR_INVALID_ARG;

    esp_err_t error = bmx280_probe(bmx280) || bmx280_reset(bmx280);

    if (error == ESP_OK)
    {
        // Give the sensor 10 ms delay to reset.
        vTaskDelay(pdMS_TO_TICKS(10));

        // Read calibration data.
        bmx280_calibrate(bmx280);

        bmx280_setMode(bmx280, BMX280_MODE_CYCLE);

        ESP_LOGI("bmx280", "Dumping calibration...");
        ESP_LOG_BUFFER_HEX("bmx280", &bmx280->cmps, sizeof(bmx280->cmps));
    }
    else
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(error);
    }

    return error;
}

esp_err_t bmx280_configure(bmx280_t *bmx280, bmx280_config_t *cfg)
{
    if (bmx280 == NULL || cfg == NULL)
        return ESP_ERR_INVALID_ARG;
    if (!bmx280_validate(bmx280))
        return ESP_ERR_INVALID_STATE;

    // Always set ctrl_meas first.
    uint8_t num = (cfg->t_sampling << 5) | (cfg->p_sampling << 2) | BMX280_MODE_SLEEP;
    esp_err_t err = bmx280_write(bmx280, BMX280_REG_MESCTL, &num, sizeof num);

    if (err)
        return err;

    // We can set cfg now.
    num = (cfg->t_standby << 5) | (cfg->iir_filter << 2);
    err = bmx280_write(bmx280, BMX280_REG_CONFIG, &num, sizeof num);

    if (err)
        return err;

    {
        num = cfg->h_sampling;
        err = bmx280_write(bmx280, BMX280_REG_HUMCTL, &num, sizeof(num));

        if (err)
            return err;
    }

    // f = 0;
    return ESP_OK;
}

esp_err_t bmx280_setMode(bmx280_t *bmx280, bmx280_mode_t mode)
{
    esp_err_t err;

    // if ((err = bmx280_read(bmx280, BMX280_REG_MESCTL, &ctrl_mes, 1)) != ESP_OK)
    //   return err;

    // ctrl_mes = (ctrl_mes & (~3)) | mode;

    uint8_t ctrl_mes = 0x01;
    err = bmx280_write(bmx280, BMX280_REG_HUMCTL, &ctrl_mes, 1);

    if (err != ESP_OK)
    {
        return err;
    }

    ctrl_mes = 0x27;
    err = bmx280_write(bmx280, BMX280_REG_MESCTL, &ctrl_mes, 1);

    return err;
}

esp_err_t bmx280_getMode(bmx280_t *bmx280, bmx280_mode_t *mode)
{
    uint8_t ctrl_mes;
    esp_err_t err;

    if ((err = bmx280_read(bmx280, BMX280_REG_MESCTL, &ctrl_mes, 1)) != ESP_OK)
        return err;

    ctrl_mes &= 3;

    switch (ctrl_mes)
    {
    default:
        *mode = ctrl_mes;
        break;
    case (BMX280_MODE_FORCE + 1):
        *mode = BMX280_MODE_FORCE;
        break;
    }

    return ESP_OK;
}

bool bmx280_isSampling(bmx280_t *bmx280)
{
    uint8_t status;
    if (bmx280_read(bmx280, BMX280_REG_STATUS, &status, 1) == ESP_OK)
        return (status & (1 << 3)) != 0;
    else
        return false;
}

float bme280_compenstate_T(bmx280_t *sens, uint8_t *data)
{
    printf("T: %02X %02X %02X\n", data[0], data[1], data[2]);
    int32_t var1, var2;
    int32_t adc_T = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | data[2];
    if (adc_T == 0x800000)
    { // value in case temp measurement was disabled
        return ESP_FAIL;
    }
    adc_T >>= 4;

    var1 = ((((adc_T >> 3) - ((int32_t)sens->cmps.T1 << 1))) * ((int32_t)sens->cmps.T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)sens->cmps.T1)) * ((adc_T >> 4) - ((int32_t)sens->cmps.T1))) >> 12) * ((int32_t)sens->cmps.T3)) >> 14;

    sens->t_fine = var1 + var2;
    return ((sens->t_fine * 5 + 128) >> 8) / 100.0;
}

float bme280_compenstate_P(bmx280_t *sens, uint8_t *data)
{
    printf("P: %02X %02X %02X\n", data[0], data[1], data[2]);
    int32_t adc_P = (data[0] << 16) | (data[1] << 8) | data[2];
    if (adc_P == 0x800000)
    { // value in case pressure measurement was disabled
        return ESP_FAIL;
    }
    int64_t var1, var2, p;
    adc_P >>= 4;
    var1 = ((int64_t)sens->t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)sens->cmps.P6;
    var2 = var2 + ((var1 * (int64_t)sens->cmps.P5) << 17);
    var2 = var2 + (((int64_t)sens->cmps.P4) << 35);
    var1 = ((var1 * var1 * (int64_t)sens->cmps.P3) >> 8) + ((var1 * (int64_t)sens->cmps.P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)sens->cmps.P1) >> 33;
    if (var1 == 0)
    {
        return ESP_FAIL; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)sens->cmps.P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)sens->cmps.P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)sens->cmps.P7) << 4);
    p = p >> 8; // /256
    return (float)p / 100;
}

float bme280_compenstate_H(bmx280_t *sens, uint8_t *data)
{
    printf("H: %02X %02X\n", data[0], data[1]);
    uint16_t data16 = ((uint16_t)data[0] << 8) | data[1];
    float temp = 0.0;
    int32_t adc_H = data16;
    if (adc_H == 0x8000)
    { // value in case humidity measurement was disabled
        return ESP_FAIL;
    }
    int32_t v_x1_u32r;
    v_x1_u32r = (sens->t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)sens->cmps.H4) << 20) - (((int32_t)sens->cmps.H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)sens->cmps.H6)) >> 10) * (((v_x1_u32r * ((int32_t)sens->cmps.H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)sens->cmps.H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)sens->cmps.H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    return (v_x1_u32r >> 12) / 1024.0;
}

esp_err_t bmx280_readout(bmx280_t *bmx280, float *temperature, float *pressure, float *humidity)
{
    if (bmx280 == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (!bmx280_validate(bmx280))
    {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t buffer[3];
    esp_err_t error;

    if (temperature)
    {
        if ((error = bmx280_read(bmx280, BMX280_REG_TEMP_MSB, buffer, 3)) != ESP_OK)
        {
            return error;
        }

        *temperature = bme280_compenstate_T(bmx280, buffer);
    }

    if (pressure)
    {
        if ((error = bmx280_read(bmx280, BMX280_REG_PRES_MSB, buffer, 3)) != ESP_OK)
        {
            return error;
        }

        *pressure = bme280_compenstate_P(bmx280, buffer);
    }

    {
        if (humidity)
        {
            if ((error = bmx280_read(bmx280, BMX280_REG_HUMI_MSB, buffer, 2)) != ESP_OK)
            {
                return error;
            }

            *humidity = bme280_compenstate_H(bmx280, buffer);
        }
    }

    return ESP_OK;
}