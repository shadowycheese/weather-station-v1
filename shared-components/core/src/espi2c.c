#include "models.h"
#include "espi2c.h"

i2c_config_t create_i2c_input_configration(int sdaPin, int sclPin, int freq)
{
    i2c_config_t config = {};

    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = sdaPin;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_io_num = sclPin;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = freq;

    return config;
}
