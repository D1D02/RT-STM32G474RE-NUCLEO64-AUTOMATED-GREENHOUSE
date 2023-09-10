#ifndef GROUNDSENSOR_H
#define GROUNDSENSOR_H

#include "ch.h"
#include "hal.h"
#include "ssd1306.h"
#include "chprintf.h"
#include "stdio.h"

static const I2CConfig i2ccfg = {
  .timingr = 0x10,
  .cr1 = 0,
  .cr2 = 1,
};

static const SSD1306Config ssd1306cfg = {
  &I2CD4,
  &i2ccfg,
  SSD1306_SAD_0X78,
};


void startGroundSensor(void);
int* wSensorInit(void);
int getSun(void);

#endif
