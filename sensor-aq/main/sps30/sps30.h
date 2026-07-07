/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef SPS30_H
#define SPS30_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SPS30_MAX_SERIAL_LEN 32
#define SPS30_ERR_NOT_ENOUGH_DATA (-1)
#define SPS30_ERR_STATE_MASK (0x100)
#define SPS30_IS_ERR_STATE(err_code) (((err_code) | 0xff) == 0x1ff)
#define SPS30_GET_ERR_STATE(err_code) ((err_code) & 0xff)

    struct sps30_measurement
    {
        float mc_1p0;
        float mc_2p5;
        float mc_4p0;
        float mc_10p0;
        float nc_0p5;
        float nc_1p0;
        float nc_2p5;
        float nc_4p0;
        float nc_10p0;
        float typical_particle_size;
    };

    struct sps30_version_information
    {
        uint8_t firmware_major;
        uint8_t firmware_minor;
        uint8_t hardware_revision;
        uint8_t shdlc_major;
        uint8_t shdlc_minor;
    };

    int16_t sps30_probe(void);

    int16_t sps30_get_serial(char *serial);

    int16_t sps30_start_measurement(void);

    int16_t sps30_stop_measurement(void);

    int16_t sps30_read_measurement(struct sps30_measurement *measurement);

    int16_t sps30_sleep(void);

    int16_t sps30_wake_up(void);

    int16_t sps30_get_fan_auto_cleaning_interval(uint32_t *interval_seconds);

    int16_t sps30_set_fan_auto_cleaning_interval(uint32_t interval_seconds);

    int16_t sps30_get_fan_auto_cleaning_interval_days(uint8_t *interval_days);

    int16_t sps30_set_fan_auto_cleaning_interval_days(uint8_t interval_days);

    int16_t sps30_start_manual_fan_cleaning(void);

    int16_t sps30_read_version(struct sps30_version_information *version_information);

    int16_t sps30_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SPS30_H */