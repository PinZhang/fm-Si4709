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

#include "hardware_legacy/fm.h"
#include "cutils/log.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>

#define LOG_TAG "FM_HW"

struct dev_state_t
{
    int power_state;
    int seek_state;
};

struct rssi_snr_t
{
    uint8_t curr_rssi;
    uint8_t curr_rssi_th;
    uint8_t curr_snr;
};

struct device_id
{
    uint8_t part_number;
    uint16_t manufact_number;
};

struct chip_id
{
    uint8_t chip_version;
    uint8_t device;
    uint8_t firmware_version;
};

struct sys_config2
{
    uint16_t rssi_th;
    uint8_t fm_band;
    uint8_t fm_chan_spac;
    uint8_t fm_vol;
};

struct sys_config3
{
    uint8_t smmute;
    uint8_t smutea;
    uint8_t volext;
    uint8_t sksnr;
    uint8_t skcnt;
};

struct status_rssi
{
    uint8_t rdsr;
    uint8_t stc;
    uint8_t sfbl;
    uint8_t afcrl;
    uint8_t rdss;
    uint8_t blera;
    uint8_t st;
    uint16_t rssi;
};

struct power_config
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
};

struct radio_data_t
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
};

#define NUM_SEEK_PRESETS 20

#define WAIT_OVER 0
#define SEEK_WAITING 1
#define NO_WAIT 2
#define TUNE_WAITING 4
#define RDS_WAITING 5
#define SEEK_CANCEL 6

/*dev settings*/
/*band*/
#define BAND_87500_108000_kHz 1
#define BAND_76000_108000_kHz 2
#define BAND_76000_90000_kHz 3

/*channel spacing*/
#define CHAN_SPACING_200_kHz 20 /*US*/
#define CHAN_SPACING_100_kHz 10 /*Europe,Japan*/
#define CHAN_SPACING_50_kHz 5

/*DE-emphasis Time Constant*/
#define DE_TIME_CONSTANT_50 1 /*Europe,Japan,Australia*/
#define DE_TIME_CONSTANT_75 0 /*US*/


/*****************IOCTLS******************/
/*magic no*/
#define Si4709_IOC_MAGIC 0xFA
/*max seq no*/
#define Si4709_IOC_NR_MAX 40

/*commands*/
#define Si4709_IOC_POWERUP _IO(Si4709_IOC_MAGIC, 0)
#define Si4709_IOC_POWERDOWN _IO(Si4709_IOC_MAGIC, 1)
#define Si4709_IOC_BAND_SET _IOW(Si4709_IOC_MAGIC, 2, int)
#define Si4709_IOC_CHAN_SPACING_SET _IOW(Si4709_IOC_MAGIC, 3, int)
#define Si4709_IOC_CHAN_SELECT _IOW(Si4709_IOC_MAGIC, 4, uint32_t)
#define Si4709_IOC_CHAN_GET _IOR(Si4709_IOC_MAGIC, 5, uint32_t)
#define Si4709_IOC_SEEK_UP _IOR(Si4709_IOC_MAGIC, 6, uint32_t)
#define Si4709_IOC_SEEK_DOWN _IOR(Si4709_IOC_MAGIC, 7, uint32_t)
/*VNVS:28OCT'09---- Si4709_IOC_SEEK_AUTO is disabled as of now*/
//#define Si4709_IOC_SEEK_AUTO _IOR(Si4709_IOC_MAGIC, 8, u32)
#define Si4709_IOC_RSSI_SEEK_TH_SET _IOW(Si4709_IOC_MAGIC, 9, u8)
#define Si4709_IOC_SEEK_SNR_SET _IOW(Si4709_IOC_MAGIC, 10, u8)
#define Si4709_IOC_SEEK_CNT_SET _IOW(Si4709_IOC_MAGIC, 11, u8)
#define Si4709_IOC_CUR_RSSI_GET _IOR(Si4709_IOC_MAGIC, 12, rssi_snr_t)
#define Si4709_IOC_VOLEXT_ENB _IO(Si4709_IOC_MAGIC, 13)
#define Si4709_IOC_VOLEXT_DISB _IO(Si4709_IOC_MAGIC, 14)
#define Si4709_IOC_VOLUME_SET _IOW(Si4709_IOC_MAGIC, 15, u8)
#define Si4709_IOC_VOLUME_GET _IOR(Si4709_IOC_MAGIC, 16, u8)
#define Si4709_IOC_MUTE_ON _IO(Si4709_IOC_MAGIC, 17)
#define Si4709_IOC_MUTE_OFF _IO(Si4709_IOC_MAGIC, 18)
#define Si4709_IOC_MONO_SET _IO(Si4709_IOC_MAGIC, 19)
#define Si4709_IOC_STEREO_SET _IO(Si4709_IOC_MAGIC, 20)
#define Si4709_IOC_RSTATE_GET _IOR(Si4709_IOC_MAGIC, 21, dev_state_t)
#define Si4709_IOC_RDS_DATA_GET _IOR(Si4709_IOC_MAGIC, 22, radio_data_t)
#define Si4709_IOC_RDS_ENABLE _IO(Si4709_IOC_MAGIC, 23)
#define Si4709_IOC_RDS_DISABLE _IO(Si4709_IOC_MAGIC, 24)
#define Si4709_IOC_RDS_TIMEOUT_SET _IOW(Si4709_IOC_MAGIC, 25, u32)
#define Si4709_IOC_SEEK_CANCEL _IO(Si4709_IOC_MAGIC, 26)/*VNVS:START 13-OCT'09---- Added IOCTLs for reading the device-id,chip-id,power configuration, system configuration2 registers*/
#define Si4709_IOC_DEVICE_ID_GET _IOR(Si4709_IOC_MAGIC, 27,device_id)
#define Si4709_IOC_CHIP_ID_GET _IOR(Si4709_IOC_MAGIC, 28,chip_id)
#define Si4709_IOC_SYS_CONFIG2_GET _IOR(Si4709_IOC_MAGIC,29,sys_config2)
#define Si4709_IOC_POWER_CONFIG_GET _IOR(Si4709_IOC_MAGIC,30,power_config)
#define Si4709_IOC_AFCRL_GET _IOR(Si4709_IOC_MAGIC,31,u8) /*For reading AFCRL bit, to check for a valid channel*/
#define Si4709_IOC_DE_SET _IOW(Si4709_IOC_MAGIC,32,uint8_t) /*Setting DE-emphasis Time Constant. For DE=0,TC=50us(Europe,Japan,Australia) and DE=1,TC=75us(USA)*/
#define Si4709_IOC_SYS_CONFIG3_GET _IOR(Si4709_IOC_MAGIC, 33, sys_config3)
#define Si4709_IOC_STATUS_RSSI_GET _IOR(Si4709_IOC_MAGIC, 34, status_rssi)
#define Si4709_IOC_SYS_CONFIG2_SET _IOW(Si4709_IOC_MAGIC, 35, sys_config2)
#define Si4709_IOC_SYS_CONFIG3_SET _IOW(Si4709_IOC_MAGIC, 36, sys_config3)
#define Si4709_IOC_DSMUTE_ON _IO(Si4709_IOC_MAGIC, 37)
#define Si4709_IOC_DSMUTE_OFF _IO(Si4709_IOC_MAGIC, 38)
#define Si4709_IOC_RESET_RDS_DATA _IO(Si4709_IOC_MAGIC, 39)

/*****************************************/

#define THE_DEVICE "/dev/fmradio"

static int radioEnabled = 0;
static int lastFreq = 0;

static int send_signal(int signal)
{
  int rt, fd;
  fd = open(THE_DEVICE, O_RDWR);
  if (fd < 0)
    return -1;

  rt = ioctl(fd, signal);

  close(fd);
  return rt;
}

static int send_signal_with_value(int signal, int value)
{
  int rt, fd;
  fd = open(THE_DEVICE, O_RDWR);
  if (fd < 0)
    return -1;

  rt = ioctl(fd, signal, &value);

  close(fd);
  return rt;
}

int fm_exists()
{
  int fd;
  fd = open(THE_DEVICE, O_RDWR);
  if (fd < 0)
    return 0;

  close(fd);
  return 1;
}

int fm_on()
{
  LOGV("%s", __func__);

  if (1 == radioEnabled)
  {
    return 0;
  }

  int ret;
  ret = send_signal(Si4709_IOC_POWERUP);

  if (ret < 0)
  {
    LOGE("%s: IOCTL Si4709_IOC_POWERUP failed %d", __func__, ret);
    return -1;
  }

  radioEnabled = 1;

  if (lastFreq != 0)
  {
    fm_set_freq(lastFreq);
  }

  return 0;
}

int fm_off()
{
  LOGV("%s", __func__);
  if (0 == radioEnabled)
  {
    return 0;
  }

  int ret;
  ret = send_signal(Si4709_IOC_POWERDOWN);

  if (ret < 0)
  {
    LOGE("%s: IOCTL Si4709_IOC_POWERDOWN failed %d", __func__, ret);
    return -1;
  }

  radioEnabled = 0;

  return 0;
}

int fm_set_freq(int freq)
{
  LOGV("%s", __func__);

  int ret;
  ret = send_signal_with_value(Si4709_IOC_CHAN_SELECT, freq);

  if (ret < 0)
  {
    LOGE("%s: IOCTL Si4709_IOC_CHAN_SELECT failed %d", __func__, ret);
    return -1;
  }

  lastFreq = freq;
  return 0;
}

