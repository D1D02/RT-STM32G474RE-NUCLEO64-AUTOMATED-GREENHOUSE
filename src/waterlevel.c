#include "../lib/waterlevel.h"
#include "../lib/map.h"
#include "../lib/groundsensor.h"

static SSD1306Driver SSD1306D2;

int* wlevel;

#define LEDY_LINE               PAL_LINE( GPIOB, 10 ) //led rosso
#define LEDX_LINE               PAL_LINE( GPIOA, 10 ) //led verde
#define PWM_TIMER_FREQUENCY     10000
#define PWM_PERIOD              10000
#define BUFF_SIZE 20
char buff[BUFF_SIZE];

void pwmWidtchCb(PWMDriver *pwmp){
  (void)pwmp;
}

/*
 * Configures PWM Driver.
 */
static PWMConfig pwmcfg = {
  PWM_TIMER_FREQUENCY,
  PWM_PERIOD,
  pwmWidtchCb,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0,
  0
};

static THD_WORKING_AREA( waThdBuzzerLed, 1024);
static THD_FUNCTION( thdBuzzerLed, arg ) {
  (void) arg;

  wlevel = wSensorInit();

  palSetPadMode(GPIOB, 4, PAL_MODE_ALTERNATE(2));
  palSetLineMode( LEDX_LINE, PAL_MODE_OUTPUT_PUSHPULL );
  palSetLineMode( LEDY_LINE, PAL_MODE_OUTPUT_PUSHPULL );

  while(true){

  if(*wlevel<=30){
    palSetLine(LEDY_LINE);
    palClearLine(LEDX_LINE);

    pwmStart(&PWMD3, &pwmcfg);
    pwmEnablePeriodicNotification(&PWMD3);
    pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, 2500));
  while (*wlevel<=30) {
      chThdSleepMilliseconds(500);
    }
  }else {
    pwmDisableChannel(&PWMD3, 0);
    pwmStop(&PWMD3);
    palSetLine(LEDX_LINE);
    palClearLine(LEDY_LINE);
  }
    chThdSleepMilliseconds(500);
  }

}

static THD_WORKING_AREA(waOledDisplay, 1024);
static THD_FUNCTION(OledDisplay, arg) {
  (void)arg;



  chRegSetThreadName("Oled");

  palSetLineMode(PAL_LINE(GPIOB, 8U), PAL_MODE_ALTERNATE(4) |
                     PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |
                     PAL_STM32_PUPDR_PULLUP);
      palSetLineMode(PAL_LINE(GPIOB, 9U), PAL_MODE_ALTERNATE(4) |
                     PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |
                     PAL_STM32_PUPDR_PULLUP);

  ssd1306ObjectInit(&SSD1306D2);

  ssd1306Start(&SSD1306D2, &ssd1306cfg1);

  ssd1306FillScreen(&SSD1306D2, 0x00);

  while (true) {
    ssd1306UpdateScreen(&SSD1306D2);
    ssd1306GotoXy(&SSD1306D2, 0, 1);
    chsnprintf(buff, BUFF_SIZE, "WATER LEVEL:");
    ssd1306Puts(&SSD1306D2, buff, &ssd1306_font_7x10, SSD1306_COLOR_WHITE);
    ssd1306GotoXy(&SSD1306D2, 0, 30);
    chsnprintf(buff, BUFF_SIZE, "%d ",*wlevel);
    ssd1306Puts(&SSD1306D2, buff, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);
    ssd1306GotoXy(&SSD1306D2, 32, 30);
    chsnprintf(buff,BUFF_SIZE, "%%");
    ssd1306Puts(&SSD1306D2, buff, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

ssd1306DrawRectangle(&SSD1306D2, 25,50,128,40,  SSD1306_COLOR_BLACK);

     if((*wlevel>30) && (*wlevel<40)){
          ssd1306UpdateScreen(&SSD1306D2);
          ssd1306GotoXy(&SSD1306D2, 25, 50);
          chsnprintf(buff, BUFF_SIZE, " ATTENTION");
          ssd1306Puts(&SSD1306D2, buff, &ssd1306_font_7x10,  SSD1306_COLOR_WHITE);

    }
    else if(*wlevel<30){
              ssd1306GotoXy(&SSD1306D2, 25, 50);
              chsnprintf(buff, BUFF_SIZE, " LOW LEVEL");
              ssd1306Puts(&SSD1306D2, buff, &ssd1306_font_7x10,  SSD1306_COLOR_WHITE);

    }
    else {
      ssd1306GotoXy(&SSD1306D2, 25, 50);
      chsnprintf(buff, BUFF_SIZE, "GOOD LEVEL");
      ssd1306Puts(&SSD1306D2, buff, &ssd1306_font_7x10,  SSD1306_COLOR_WHITE);
    }
     ssd1306UpdateScreen(&SSD1306D2);
     chThdSleepMilliseconds(500);
  }


    adcStop(&ADCD1);

}


  void startWaterSensor(void){  
   chThdCreateStatic(waOledDisplay, sizeof(waOledDisplay), NORMALPRIO+1, OledDisplay, NULL);
   chThdCreateStatic(waThdBuzzerLed, sizeof(waThdBuzzerLed), NORMALPRIO + 5, thdBuzzerLed, NULL);
  }
 









