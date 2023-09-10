#include "../lib/dht11.h"

int TEMP=0, HR=0, CHECK_SUM, tmp, bit_counter = 0;

static icucnt_t widths [40];
static void icuwidthcb(ICUDriver *icup) {
  icucnt_t width = icuGetWidthX(icup);
  if(width >= DHT_START_BIT_WIDTH){
    bit_counter = 0;
  }
  else{
    widths[bit_counter] = width;
    if(width > DHT_LOW_BIT_WIDTH){
      tmp |= (1 << (7 - (bit_counter % 8)));
    }
    else{
      tmp &= ~(1 << (7 - (bit_counter % 8)));
    }
    if(bit_counter == 7)
      HR = tmp;
    if(bit_counter == 23)
      TEMP = tmp;
    if(bit_counter == 39)
      CHECK_SUM = tmp;
    bit_counter++;
  }
}

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  ICU_FREQUENCY,
  NULL,
  icuwidthcb,
  NULL,
  ICU_CHANNEL_1,
  2U,
  0xFFFFFFFFU
};

static THD_WORKING_AREA(waDht, 128);
static THD_FUNCTION(Dht, arg) {
  (void)arg;
  chRegSetThreadName("Dht");
  while (true) {

       palSetPadMode(GPIOB, 2U, PAL_MODE_OUTPUT_PUSHPULL);
            palWritePad(GPIOB, 2U, PAL_LOW);
            chThdSleepMicroseconds(MCU_REQUEST_WIDTH);
            palWritePad(GPIOB, 2U, PAL_HIGH);

            palSetPadMode(GPIOB, 2U, PAL_MODE_ALTERNATE(2));
            icuStart(&ICUD5, &icucfg);
            icuStartCapture(&ICUD5);
            icuEnableNotifications(&ICUD5);
            chThdSleepMilliseconds(700);

            icuStopCapture(&ICUD5);
            icuStop(&ICUD5);
  }
}

void dhtInit(void){
  chThdCreateStatic(waDht, sizeof(waDht), NORMALPRIO, Dht, NULL);
}

int getTemp(void){
  return TEMP;
}
int getHum(void){
  return HR;
}
