/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>

#ifndef _FM_H
#define _FM_H

#if __cplusplus
extern "c" {
#endif

typedef struct
{
    uint16_t dsmute :1;
    uint16_t dmute:1;
    uint16_t mono:1;
    uint16_t rds_mode:1;
    uint16_t sk_mode:1;
    uint16_t seek_up:1;
    uint16_t seek:1;
    uint16_t power_disable:1;
    uint16_t power_enable:1;
} power_config;

typedef struct
{
    int power_state;
    int seek_state;
} dev_state_t;

typedef struct
{
    uint8_t curr_rssi;
    uint8_t curr_rssi_th;
    uint8_t curr_snr;
} rssi_snr_t;

typedef struct
{
    uint8_t part_number;
    uint16_t manufact_number;
} device_id;

typedef struct
{
    uint8_t chip_version;
    uint8_t device;
    uint8_t firmware_version;
} chip_id;

typedef struct
{
    uint16_t rssi_th;
    uint8_t fm_band;
    uint8_t fm_chan_spac;
    uint8_t fm_vol;
} sys_config2;

typedef struct
{
    uint8_t smmute;
    uint8_t smutea;
    uint8_t volext;
    uint8_t sksnr;
    uint8_t skcnt;
} sys_config3;

typedef struct
{
    uint8_t rdsr;
    uint8_t stc;
    uint8_t sfbl;
    uint8_t afcrl;
    uint8_t rdss;
    uint8_t blera;
    uint8_t st;
    uint16_t rssi;
} status_rssi;

typedef struct
{
    uint16_t rdsa;
    uint16_t rdsb;
    uint16_t rdsc;
    uint16_t rdsd;
    uint8_t curr_rssi;
    uint32_t curr_channel;
    uint8_t blera;
    uint8_t blerb;
    uint8_t blerc;
    uint8_t blerd;
} radio_data_t;

int fm_exists();

int fm_set_freq(int freq);

int fm_get_freq();

int fm_on();

int fm_off();

int fm_seek_down();

int fm_seek_up();

int fm_cancel_seek();

int fm_set_band(int band);

int fm_get_power_state();

int fm_get_vol();

int fm_set_vol(int vol);

int fm_get_power_config();

int fm_mute_on();

int fm_mute_off();

#if __cplusplus
};  // extern "C"
#endif

#endif  // _FM_H
