#ifndef WATERLEVEL_H
#define WATERLEVEL_H

#include "ch.h"
#include "hal.h"
#include "ssd1306.h"
#include "chprintf.h"
#include "stdio.h"

static const I2CConfig i2ccfg1 = {
  .timingr = 0x10,
  .cr1 = 0,
  .cr2 = 1,
};

static const SSD1306Config ssd1306cfg1 = {
  &I2CD1,
  &i2ccfg1,
  SSD1306_SAD_0X78,
};

void startWaterSensor(void);

#endif
