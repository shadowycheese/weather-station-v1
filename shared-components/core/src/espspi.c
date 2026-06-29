#include "models.h"
#include "espspi.h"

void init_spi_slave(int spiHost, int mosi, int miso, int sclk, int cs, int handshake)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << handshake);
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)handshake, 0);

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = mosi;
    buscfg.miso_io_num = miso;
    buscfg.sclk_io_num = sclk;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;

    spi_slave_interface_config_t slvcfg = {};
    slvcfg.mode = 0; // SPI Mode 0 (CPOL=0, CPHA=0)
    slvcfg.spics_io_num = cs;
    slvcfg.queue_size = 3;
    slvcfg.flags = 0;

    spi_slave_initialize(spiHost, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
}

spi_device_handle_t init_spi_master(int spiHost, int mosi, int miso, int sclk, int cs, int handshake)
{
    spi_device_handle_t spi_handle;

    // 1. Configure Handshake Input Pin
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE; // You can use polling or a real ISR here
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << handshake);
    gpio_config(&io_conf);

    // 2. Configure SPI Master Bus
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = mosi;
    buscfg.miso_io_num = miso;
    buscfg.sclk_io_num = sclk;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    spi_bus_initialize(spiHost, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 1000000; // Start safe at 1 MHz
    devcfg.mode = 0;
    devcfg.spics_io_num = cs;
    devcfg.queue_size = 3;

    spi_bus_add_device(spiHost, &devcfg, &spi_handle);

    return spi_handle;
}

void send_data_to_master(int host, int handshake, uint8_t *data_buffer, size_t length)
{
    spi_slave_transaction_t t = {};
    t.length = length * 8;
    t.tx_buffer = data_buffer;
    t.rx_buffer = NULL;

    spi_slave_queue_trans(host, &t, portMAX_DELAY);

    gpio_set_level((gpio_num_t)handshake, 1);

    spi_slave_transaction_t *ret_trans;

    spi_slave_get_trans_result(host, &ret_trans, portMAX_DELAY);

    gpio_set_level((gpio_num_t)handshake, 0);
}