#ifndef ESPSPI_H
#define ESPSPI_H

#include <stdint.h>
#include "driver/spi_slave.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    spi_device_handle_t init_spi_master(int spiHost, int mosi, int miso, int sclk, int cs, int handshake);

    void init_spi_slave(int spiHost, int mosi, int miso, int sclk, int cs, int handshake);
    void send_data_to_master(int host, int handshake, uint8_t *data_buffer, size_t length);

#ifdef __cplusplus
}
#endif

#endif
