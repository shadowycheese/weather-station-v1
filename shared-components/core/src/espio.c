#include "models.h"
#include "espio.h"

esp_err_t configure_output_pin(gpio_num_t pin)
{
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    return gpio_config(&io_conf);
}

esp_err_t configure_input_pin(gpio_num_t pin, gpio_int_type_t interruptType)
{
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    return gpio_config(&io_conf);
}

esp_err_t disable_pin(gpio_num_t pin)
{
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_DISABLE);
    gpio_pulldown_dis(GPIO_NUM_4);
    gpio_pullup_dis(GPIO_NUM_4);

    return ESP_OK;
}