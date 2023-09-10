#ifndef SERVO_H
#define SERVO_H

#include "ch.h"
#include "hal.h"
#include "ssd1306.h"
#include "chprintf.h"
#include "stdio.h"

#define PWM_TIMER_FREQUENCY     1000000                             
#define PWM_PERIOD              (PWM_TIMER_FREQUENCY * 20 / 1000)  




void servoInit(void);


#endif
