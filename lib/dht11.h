#ifndef     DHT11_H
#define     DHT11_H

#include "ch.h"
#include "hal.h"
#include "stdio.h"

#define    MCU_REQUEST_WIDTH                     18000
#define    DHT_ERROR_WIDTH                         200
#define    DHT_START_BIT_WIDTH                      80
#define    DHT_LOW_BIT_WIDTH                        28
#define    DHT_HIGH_BIT_WIDTH                       70
#define    ICU_FREQUENCY                       1000000

void dhtInit(void);
int getTemp(void);
int getHum(void);

#endif
