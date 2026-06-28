#include "models.h"
#include "espio.h"

esp_err_t configure_output_pin(int pin)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    return gpio_config(&io_conf);
}

esp_err_t configure_input_pin(int pin)
{
    // Configure the power pin
    return ESP_OK;
}

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