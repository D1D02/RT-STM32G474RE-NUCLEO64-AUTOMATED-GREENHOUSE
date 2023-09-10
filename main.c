/*
 NeaPolis Innovation Summer Campus 2023 team 15 project
 Copyright (C) 2023   Di Donato Andrea

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/*
    RT-STM32G474RE-NUCLEO64-AUTOMATED-GREENHOUSE
    A partial automated greenhouse, with servo for humidity release,
    a pump to water plants when terrain humidity is low, a led strip for night time,
    3 oleds to display various information like: water level in the tank, terrain
    humidity and ambiental data from DHT11.
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "ssd1306.h"
#include "stdio.h"
#include "../lib/groundsensor.h"
#include "../lib/waterlevel.h"
#include "../lib/dht11.h"
#include "../lib/servo.h"
#include "image.c"

#define BUFF_SIZE   20
char buff2[BUFF_SIZE];

uint8_t chk=0;

static SSD1306Driver SSD1306D2;

void  weather(void);
void dhtData(void);
void nostroMotto(void);

static const I2CConfig i2ccfg2 = {
  .timingr = 0x10,
  .cr1 = 0,
  .cr2 = 1,
};

static const SSD1306Config ssd1306cfg2 = {
  &I2CD3,
  &i2ccfg2,
  SSD1306_SAD_0X78,
};

static THD_WORKING_AREA(waOledMain, 1024);
static THD_FUNCTION(OledMain, arg) {
  (void)arg;

  chRegSetThreadName("OledMain");
  palSetLineMode(PAL_LINE(GPIOC, 8U), PAL_MODE_ALTERNATE(8) |
                   PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |
                   PAL_STM32_PUPDR_PULLUP);
    palSetLineMode(PAL_LINE(GPIOC, 9U), PAL_MODE_ALTERNATE(8) |
                   PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST |
                   PAL_STM32_PUPDR_PULLUP);

  ssd1306ObjectInit(&SSD1306D2);
  ssd1306Start(&SSD1306D2, &ssd1306cfg2);
  ssd1306FillScreen(&SSD1306D2, 0x00);


  while (true) {

      dhtData();
      weather();
      nostroMotto();

    chThdSleepMilliseconds(250);
  }

}

int main(void) {

  halInit();
  chSysInit();
  startGroundSensor();
  startWaterSensor();
  dhtInit();
  servoInit();


  chThdCreateStatic(waOledMain, sizeof(waOledMain), NORMALPRIO, OledMain, NULL);

  while (true) {
    if( palReadLine( LINE_BUTTON ) == PAL_HIGH ) {
          chThdSleepMilliseconds(5);
          while( palReadLine( LINE_BUTTON ) == PAL_HIGH ) {
            chThdSleepMilliseconds(10);
            chk=1;
          }

        }
    chThdSleepMilliseconds(10);
}
}

void dhtData(void){
      ssd1306FillScreen(&SSD1306D2, 0x00);
      ssd1306GotoXy(&SSD1306D2, 0, 1);
      chsnprintf(buff2, BUFF_SIZE, "DHT11 DATA:");
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

      ssd1306GotoXy(&SSD1306D2, 0, 30);
      chsnprintf(buff2, BUFF_SIZE, "%dC %d",getTemp(), getHum());
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

      ssd1306GotoXy(&SSD1306D2, 72, 30);
      chsnprintf(buff2,BUFF_SIZE, "%%");
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

      ssd1306UpdateScreen(&SSD1306D2);
      chThdSleepMilliseconds(3000);
}

void nostroMotto(void){
      ssd1306FillScreen(&SSD1306D2, 0x00);

      ssd1306GotoXy(&SSD1306D2, 0, 1);
      chsnprintf(buff2, BUFF_SIZE, "Uomini forti, ");
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_7x10, SSD1306_COLOR_WHITE);

      ssd1306GotoXy(&SSD1306D2, 0, 15);
      chsnprintf(buff2, BUFF_SIZE, "destini forti.");
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_7x10, SSD1306_COLOR_WHITE);

      ssd1306GotoXy(&SSD1306D2, 0, 30);
      chsnprintf(buff2, BUFF_SIZE, "Uomini deboli, ");
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_7x10, SSD1306_COLOR_WHITE);

      ssd1306GotoXy(&SSD1306D2, 0, 45);
      chsnprintf(buff2, BUFF_SIZE, "destini deboli.");
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_7x10, SSD1306_COLOR_WHITE);

      ssd1306UpdateScreen(&SSD1306D2);
      chThdSleepMilliseconds(3000);
      ssd1306FillScreen(&SSD1306D2, 0x00);

      ssd1306GotoXy(&SSD1306D2, 15, 10);
      chsnprintf(buff2, BUFF_SIZE, "NON C'E' ");
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

      ssd1306GotoXy(&SSD1306D2, 0, 35);
      chsnprintf(buff2, BUFF_SIZE, "ALTRA");
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

      ssd1306GotoXy(&SSD1306D2, 60, 35);
      chsnprintf(buff2, BUFF_SIZE, "STRADA");
      ssd1306Puts(&SSD1306D2, buff2, &ssd1306_font_11x18, SSD1306_COLOR_WHITE);

      ssd1306UpdateScreen(&SSD1306D2);
      chThdSleepMilliseconds(3000);
}

void  weather(void){

    ssd1306FillScreen(&SSD1306D2, 0x00);
    int x, y, c, offset=0;

    if(getSun()>40){
        offset = 0 ;
    }else{
        offset = 4096;
    }

     x = 0;
     y = 0;
     c = 0 ;

      for(y = 0; y<64; y++){
            for(x = 0; x<64; x++){
                ssd1306DrawPixel(&SSD1306D2, x+32, y, mat[c+offset]);
                c++;
              }
            ssd1306UpdateScreen(&SSD1306D2);
          }
        chThdSleepMilliseconds(3000);
       chk=0;
}
