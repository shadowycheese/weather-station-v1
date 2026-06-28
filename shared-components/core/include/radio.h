#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

#define NET_H2_SENSOR_PAN_ID 0x2222
#define NET_H2_SENSOR_CHANNEL 26 // Your isolated Wi-Fi guard band!

#define NET_H2_SENSOR_TEMP_1 0x0001
#define NET_H2_SENSOR_TEMP_2 0x0002
#define NET_H2_SENSOR_AIR_QUALITY 0x0003
#define NET_H2_SENSOR_UV 0x0004
#define NET_H2_SENSOR_RAIN_GAUGE 0x0005

#ifdef __cplusplus
#include <cstring> // Pulled when main.cpp reads this file
#else
#include <string.h> // Pulled when models.c reads this file
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    extern const uint8_t NET_P4_MAC[6];
    extern const uint8_t NET_H2_SENSOR_TEMP_1_MAC[6];
    extern const uint8_t NET_H2_RECEIVER_MAC[6];

    extern int ieee_802154_transmit_sensor_data(sensor_data_t *sensorData);

#ifdef __cplusplus
}
#endif

#endif
