#include "sgp30/sgp30.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "math.h"
#include <string.h>

static const char *TAG = "sgp30";
static i2c_master_dev_handle_t s_spg30_i2c_dev_handle;

// SGP30 commands
#define SGP30_CMD_INIT_AIR_QUALITY 0x2003
#define SGP30_CMD_MEASURE_AIR_QUALITY 0x2008
#define SGP30_CMD_GET_SERIAL_ID 0x3682
#define SGP30_CMD_SET_HUMIDITY 0x2061

#define SGP30_MEASURE_TIME_MS 20

static esp_err_t sgp30_write_cmd(uint16_t cmd)
{
    uint8_t write_buffer[2] = {(uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF)};

    esp_err_t ret = i2c_master_transmit(s_spg30_i2c_dev_handle, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(100));
    return ret;
}

esp_err_t sgp30_init(i2c_master_bus_handle_t bus_handle, uint16_t address, int speed)
{
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = speed,
        .scl_wait_us = 40000};
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &s_spg30_i2c_dev_handle));

    // Send init air quality command
    esp_err_t ret = sgp30_write_cmd(SGP30_CMD_INIT_AIR_QUALITY);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SGP30");
        return ret;
    }
    ESP_LOGI(TAG, "device_create for SGP30 sensors on ADDR %X", address);

    return ESP_OK;
}

uint8_t sgp30_crc8(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF; // SGP30 initialization vector
    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0x31; // Sensirion CRC polynomial
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

esp_err_t sgp30_set_humidity(float absolute_humidity_g_m3)
{
    if (absolute_humidity_g_m3 > 256.0f)
    {
        absolute_humidity_g_m3 = 256.0f;
    }
    if (absolute_humidity_g_m3 < 0.0f)
    {
        absolute_humidity_g_m3 = 0.0f;
    }

    uint16_t humidity_fixed_point = (uint16_t)(absolute_humidity_g_m3 * 256.0f);

    uint8_t tx_buffer[5];
    tx_buffer[0] = (SGP30_CMD_SET_HUMIDITY) >> 8;
    tx_buffer[1] = (SGP30_CMD_SET_HUMIDITY) & 0xFF;
    tx_buffer[2] = (uint8_t)(humidity_fixed_point >> 8);
    tx_buffer[3] = (uint8_t)(humidity_fixed_point & 0xFF);

    tx_buffer[4] = sgp30_crc8(&tx_buffer[2], 2);

    esp_err_t ret = i2c_master_transmit(s_spg30_i2c_dev_handle, tx_buffer, sizeof(tx_buffer), pdMS_TO_TICKS(100));

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write humidity compensation data: %s", esp_err_to_name(ret));
    }

    return ret;
}

esp_err_t sgp30_read_measurements(uint16_t *eco2, uint16_t *tvoc)
{
    uint8_t data[6];

    esp_err_t ret = sgp30_write_cmd(SGP30_CMD_MEASURE_AIR_QUALITY);
    if (ret != ESP_OK)
    {
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(SGP30_MEASURE_TIME_MS));

    ret = i2c_master_receive(s_spg30_i2c_dev_handle, data, 6, pdMS_TO_TICKS(100));

    if (ret != ESP_OK)
    {
        return ret;
    }

    *eco2 = (data[0] << 8) | data[1];
    if (sgp30_crc8(&data[0], 2) != data[2])
    {
        *eco2 = 111;
        return ESP_ERR_INVALID_CRC;
    }

    *tvoc = (data[3] << 8) | data[4];
    if (sgp30_crc8(&data[3], 2) != data[5])
    {
        *tvoc = 111;
        return ESP_ERR_INVALID_CRC;
    }

    return ret;
}

esp_err_t sgp30_get_serial_id(uint8_t *serial_id)
{
    if (serial_id == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[6];

    esp_err_t ret = sgp30_write_cmd(SGP30_CMD_GET_SERIAL_ID);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = i2c_master_receive(s_spg30_i2c_dev_handle, data, 6, pdMS_TO_TICKS(100));

    if (ret == ESP_OK)
    {
        memcpy(serial_id, data, 6);
    }

    return ret;
}
