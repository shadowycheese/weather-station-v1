#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

#define NET_H2_SENSOR_PAN_ID 0x2222
#define NET_H2_SENSOR_CHANNEL 26 // Your isolated Wi-Fi guard band!

#ifdef __cplusplus
#include <cstring> // Pulled when main.cpp reads this file
#else
#include <string.h> // Pulled when models.c reads this file
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    extern int ieee_802154_transmit_sensor_data(uint16_t source, sensor_data_t *sensorData);

#ifdef __cplusplus
}
#endif

#endif
