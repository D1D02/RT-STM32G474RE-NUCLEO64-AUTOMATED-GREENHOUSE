#include "../lib/groundsensor.h"
#include "../lib/map.h"

static SSD1306Driver SSD1306D1;



#define BUFF_SIZE   20
char buff1[BUFF_SIZE];
float igrometer, water, sun;
int humidityLevelPercentage=0, waterLevelPercentage=0, sunPercentage=0;

int* wSensorInit(void){
   return (int*)(&waterLevelPercentage);
}

#define VOLTAGE_RES            ((float)3.3/4096)

#define MSG_ADC_OK               0x1337
#define MSG_ADC_KO               0x7331
static thread_reference_t trp = NULL;

static void adccallback(ADCDriver *adcp) {
  if (adcIsBufferComplete(adcp)) {
    chSysLockFromISR();
    chThdResumeI(&trp, (msg_t) MSG_ADC_OK );
    chSysUnlockFromISR();
  }
}


static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {
  (void)adcp;
  (void)err;
  chSysLockFromISR();
  chThdResumeI(&trp, (msg_t) MSG_ADC_KO );
  chSysUnlockFromISR();
}

#define ADC_GRP_NUM_CHANNELS        4
#define ADC_GRP_BUF_DEPTH           16
static adcsample_t samples[ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH];

static const ADCConversionGroup adcgrpcfg = {
          .circular     = false,
          .num_channels = ADC_GRP_NUM_CHANNELS,
          .end_cb       = adccallback,
          .error_cb     = adcerrorcallback,
          .cfgr         = ADC_CFGR_CONT,
          .cfgr2        = 0U,
          .tr1          = ADC_TR_DISABLED,
          .tr2          = ADC_TR_DISABLED,
          .tr3          = ADC_TR_DISABLED,
          .awd2cr       = 0U,
          .awd3cr       = 0U,
          .smpr         = {
            ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_247P5) |
            ADC_SMPR1_SMP_AN2(ADC_SMPR_SMP_247P5)|
            ADC_SMPR1_SMP_AN3(ADC_SMPR_SMP_247P5),
            0U
          },
          .sqr          = {
            ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2)|ADC_SQR1_SQ3_N(ADC_CHANNEL_IN15),
            0U,
            0U,
            0U
          }
        };

static float converted[ADC_GRP_NUM_CHANNELS];


static THD_WORKING_AREA( waThdHumidityLevelSensor, 1024);
static THD_FUNCTION( thdHumidityLevelSensor, arg ) {
  (void) arg;

   palSetPadMode(GPIOA, 0U, PAL_MODE_INPUT_ANALOG);
   palSetPadMode(GPIOA, 1U, PAL_MODE_INPUT_ANALOG);
   palSetPadMode(GPIOB, 0U, PAL_MODE_INPUT_ANALOG);

   adcStart(&ADCD1, NULL);

   while ( true ){
    msg_t msg;
    int i;

    chSysLock();
    adcStartConversionI(&ADCD1, &adcgrpcfg, samples, ADC_GRP_BUF_DEPTH);

    msg = chThdSuspendS(&trp);

    chSysUnlock();

    if( msg == MSG_ADC_KO ) {
      continue;
    }

    for( i = 0; i < ADC_GRP_NUM_CHANNELS; i++ ) {
      converted[i] = 0.0f;
    }

    for( i = 0; i < ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH; i++ ) {
      converted[ i % ADC_GRP_NUM_CHANNELS] += (float) samples[i] * VOLTAGE_RES;
    }

    for( i = 0; i < ADC_GRP_NUM_CHANNELS; i++ ) {
      converted[i] /= ADC_GRP_BUF_DEPTH;
    }

    igrometer = converted[0];
    water = converted[1];
    sun = converted[2];

    humidityLevelPercentage = map(igrometer,1.917128205, 2.5, 0, 100);
    waterLevelPercentage = map(water, 1.792401313, 2.2, 0, 100);
    sunPercentage = map(sun, 0, 3.29, 0, 100);

    chThdSleepMilliseconds(500);
  }
  adcStop(&ADCD1);
}

static THD_WORKING_AREA(waOledGround, 1024);
static THD_FUNCTION(OledGround, arg) {
  (void)arg;

  chRegSetThreadName("OledGround");
  palSetPadMode(GPIOB, 3U, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(PAL_LINE(GPIOC, 7U), PAL_MODE_ALTERNATE(8) |
                   PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |
                   PAL_STM32_PUPDR_PULLUP);
    palSetLineMode(PAL_LINE(GPIOC, 6U), PAL_MODE_ALTERNATE(8) |
                   PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |
                   PAL_STM32_PUPDR_PULLUP);
  ssd1306ObjectInit(&SSD1306D1);
  ssd1306ObjectInit(&SSD1306D1);
  ssd1306Start(&SSD1306D1, &ssd1306cfg);



  while (true) {
    ssd1306FillScreen(&SSD1306D1, 0x00);
    ssd1306GotoXy(&SSD1306D1, 0, 1);
    chsnprintf(buff1, BUFF_SIZE, "GROUND HUMIDITY:");
    ssd1306Puts(&SSD1306D1, buff1, &ssd1306_font_7x10, SSD1306_COLOR_WHITE);

    ssd1306GotoXy(&SSD1306D1, 0, 15);
    chsnprintf(buff1, BUFF_SIZE, "%d ",humidityLevelPercentage);
    ssd1306Puts(&SSD1306D1, buff1, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

    ssd1306GotoXy(&SSD1306D1, 32, 15);
    chsnprintf(buff1,BUFF_SIZE, "%%");
    ssd1306Puts(&SSD1306D1, buff1, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

    if(humidityLevelPercentage>25){
      palClearPad(GPIOB, 3U);
    }else if(humidityLevelPercentage<=25 && waterLevelPercentage > 30){
      palSetPad(GPIOB, 3U);
      ssd1306GotoXy(&SSD1306D1, 0, 32);
      chsnprintf(buff1,BUFF_SIZE, "Watering");
      ssd1306Puts(&SSD1306D1, buff1, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

    }else if(humidityLevelPercentage<=25 && waterLevelPercentage <= 30){
      palClearPad(GPIOB, 3U);
      ssd1306GotoXy(&SSD1306D1, 0, 32);
      chsnprintf(buff1,BUFF_SIZE, "Not enough water");
      ssd1306Puts(&SSD1306D1, buff1, &ssd1306_font_7x10, SSD1306_COLOR_WHITE);
    }

    ssd1306UpdateScreen(&SSD1306D1);
    chThdSleepMilliseconds(2000);
  }

}

static THD_WORKING_AREA(waLedStrip, 1024);
static THD_FUNCTION(ledStrip, arg) {
  (void)arg;

  chRegSetThreadName("LedStrip");
  palSetPadMode(GPIOA, 7U, PAL_MODE_OUTPUT_PUSHPULL);

  while (true) {
     if(sunPercentage<30){
       palSetPad(GPIOA, 7U);
     }else{
       palClearPad(GPIOA, 7U);
     }
    chThdSleepMilliseconds(3500);
  }

}

  void startGroundSensor(void){
    chThdCreateStatic( waThdHumidityLevelSensor, sizeof( waThdHumidityLevelSensor), NORMALPRIO + 5, thdHumidityLevelSensor, (void*) NULL );
    chThdCreateStatic(waOledGround, sizeof(waOledGround), NORMALPRIO, OledGround, NULL);
    chThdCreateStatic(waLedStrip, sizeof(waLedStrip), NORMALPRIO, ledStrip, NULL);
  }
  
int getSun(void){
    return sunPercentage;
}






