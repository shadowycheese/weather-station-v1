#include "models.h"
#include "espuart.h"

esp_err_t start_uart(uart_port_t uart_num, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num, int buffer_size, int speed)
{
    uart_config_t uart_config = {
        .baud_rate = speed,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t err = uart_param_config(uart_num, &uart_config);
    if (err != ESP_OK)
    {
        return err;
    }

    err = uart_set_pin(uart_num, tx_io_num, rx_io_num, rts_io_num, cts_io_num);
    if (err != ESP_OK)
    {
        return err;
    }

    if (rts_io_num != UART_PIN_NO_CHANGE)
    {
        uart_set_line_inverse(uart_num, UART_SIGNAL_RTS_INV);
    }

    err = uart_driver_install(uart_num, buffer_size, 0, 0, NULL, 0);
    return err;
}
