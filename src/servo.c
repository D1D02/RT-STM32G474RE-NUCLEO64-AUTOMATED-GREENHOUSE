#include "../lib/servo.h"
#include "../lib/dht11.h"

void pwmWidtchCb1(PWMDriver *pwmp){
  (void)pwmp;
}

static PWMConfig pwmcfg = {
  .frequency = PWM_TIMER_FREQUENCY,
  .period = PWM_PERIOD,
  .callback = pwmWidtchCb1,
  .channels = {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  }
};

static THD_WORKING_AREA( waSG90, 1024);
static THD_FUNCTION( thdSG90, arg ) {
  (void) arg;

  palSetPadMode(GPIOA, 8, PAL_MODE_ALTERNATE(6));

  pwmStart(&PWMD1, &pwmcfg);

  int i=250; //250 1250;
  while (true) {
     pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, i));

     if(getHum()<85){
       i=1250;
     }else{
       i=250;
     }
    chThdSleepSeconds(2);
  }
  pwmDisableChannel(&PWMD1, 0);

  pwmStop(&PWMD1);

}

void servoInit() {
  chThdCreateStatic(waSG90, sizeof(waSG90), NORMALPRIO+5, thdSG90,(void*) NULL);
}

