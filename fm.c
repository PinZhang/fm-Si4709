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

#include "fm.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

#define LOG_TAG "FM_HW"

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
#define Si4709_IOC_VOLUME_SET _IOW(Si4709_IOC_MAGIC, 15, uint8_t)
#define Si4709_IOC_VOLUME_GET _IOR(Si4709_IOC_MAGIC, 16, uint8_t)
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

static int send_signal_with_value(int signal, int * value)
{
  int rt, fd;
  fd = open(THE_DEVICE, O_RDWR);
  if (fd < 0)
    return -1;

  rt = ioctl(fd, signal, value);

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
  //LOGV("%s", __func__);

  if (1 == radioEnabled)
  {
    return 0;
  }

  int ret = 0;
  ret = send_signal(Si4709_IOC_POWERUP);

  if (ret < 0)
  {
    //LOGE("%s: IOCTL Si4709_IOC_POWERUP failed %d", __func__, ret);
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
  //LOGV("%s", __func__);
  if (0 == radioEnabled)
  {
    return 0;
  }

  int ret = 0;
  ret = send_signal(Si4709_IOC_POWERDOWN);

  if (ret < 0)
  {
    //LOGE("%s: IOCTL Si4709_IOC_POWERDOWN failed %d", __func__, ret);
    return -1;
  }

  radioEnabled = 0;

  return 0;
}

int fm_set_freq(int freq)
{
  //LOGV("%s", __func__);

  // The minimal spacing is 50KHZ
  freq /= 10;

  int ret = 0;
  ret = send_signal_with_value(Si4709_IOC_CHAN_SELECT, &freq);

  if (ret < 0)
  {
    //LOGE("%s: IOCTL Si4709_IOC_CHAN_SELECT failed %d", __func__, ret);
    return -1;
  }

  lastFreq = freq;
  return 0;
}

int fm_get_freq()
{
  //LOGV("%s", __func__);
  int ret = 0;
  uint32_t freq = 0;

  ret = send_signal_with_value(Si4709_IOC_CHAN_GET, &freq);
  if (ret < 0)
  {
    //LOGE("%s: IOCTL Si4709_IOC_CHAN_GET failed %d", __func__, ret);
    return ret;
  }

  // set unit as KHZ
  freq *= 10;
  return freq;
}

int fm_seek_up()
{
  int ret = 0;
  uint32_t freq = 0;

  ret = send_signal_with_value(Si4709_IOC_SEEK_UP, &freq);
  if (ret < 0)
  {
    return ret;
  }

  return freq;
}

int fm_seek_down()
{
  int ret = 0;
  uint32_t freq = 0;

  ret = send_signal_with_value(Si4709_IOC_SEEK_DOWN, &freq);
  if (ret < 0)
  {
    return ret;
  }

  return freq;
}

int fm_cancel_seek()
{
  int ret = 0;
  ret = send_signal(Si4709_IOC_SEEK_CANCEL);

  if (ret < 0)
  {
    return -1;
  }

  return 0;
}

int fm_set_band(int band)
{
  int ret = 0;

  ret = send_signal_with_value(Si4709_IOC_BAND_SET, &band);
  if (ret < 0)
  {
    return -1;
  }

  return 0;
}

int fm_set_vol(int vol)
{
  int ret = 0;

  ret = send_signal_with_value(Si4709_IOC_VOLUME_SET, &vol);
  if (ret < 0)
  {
    return ret;
  }

  return 0;
}

int fm_get_vol()
{
  int ret;
  uint8_t vol = 0;

  ret = send_signal_with_value(Si4709_IOC_VOLUME_GET, &vol);
  if (ret < 0)
  {
    return ret;
  }

  return vol;
}

int fm_mute_on()
{
  return send_signal(Si4709_IOC_MUTE_ON);
}

int fm_mute_off()
{
  return send_signal(Si4709_IOC_MUTE_OFF);
}

int fm_get_power_config()
{
  int rt;

  int pc = 0;
  rt = send_signal_with_value(Si4709_IOC_POWER_CONFIG_GET, &pc);

  if (rt < 0)
  {
    return rt;
  }

  return pc;
}

// -1: failed
// 0: power off
// 1: power on
int fm_get_power_state()
{
  int pc = fm_get_power_config();
  if (pc < 0)
  {
    return pc;
  }

  if((pc & 256) > 0)
  {
    return 1;
  }

  return 0;
}

int fm_set_chan_spacing(int spacing)
{
  int ret = 0;
  ret = send_signal_with_value(Si4709_IOC_CHAN_SPACING_SET, &spacing);
  if (ret < 0)
  {
    return -1;
  }

  return 0;
}

void printCurrentFreq()
{
  int freq = fm_get_freq();
  printf("Current Freq: %d\n", freq);
}

int main()
{
  printf("FM Power On: %d\n", fm_get_power_state());

  printf("Turn on fm:\n");
  int ret = fm_on();
  if (ret < 0)
  {
    printf("Fail to turn on.");
    return -1;
  }

  //printf("Set Band\n");
  //fm_set_band(BAND_76000_108000_kHz);

  printCurrentFreq();

  int spacing = CHAN_SPACING_50_kHz;
  printf("Set channel spacing: %d\n", spacing);
  fm_set_chan_spacing(spacing);
  printCurrentFreq();

  int freq = 91500;
  fm_set_freq(freq);
  printCurrentFreq();

  printf("Seeking down ...\n");
  ret = fm_seek_down();
  if (freq < 0)
  {
    printf("Fail to seek down.\n");
  }
  printCurrentFreq();

  printf("Seeking up ...\n");
  ret = fm_seek_up();
  if (freq < 0)
  {
    printf("Fail to seek up.\n");
  }
  printCurrentFreq();

  // power_config pc;
  int pc = fm_get_power_config();

  if (pc < 0)
  {
    printf("Fail to get power config\n");
  }
  else
  {
    printf("Power Config: %d", pc);
  }

  printf("Set mute on\n");
  ret = fm_mute_on();
  if (ret < 0)
  {
    printf("Fail to set mute on");
  }
  pc = fm_get_power_config();

  if (pc < 0)
  {
    printf("Fail to get power config\n");
  }
  else
  {
    printf("Power Config: %d", pc);
  }

  return 0;
}

