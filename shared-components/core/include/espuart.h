#ifndef ESPUART_H
#define ESPUART_H

#include "driver/uart.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t start_uart(uart_port_t uart_num, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num, int buffer_size, int speed);

#ifdef __cplusplus
}
#endif

#endif
