/*
    Copyright 2021 Joel Svensson	svenssonjoel@yahoo.se

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"

BaseSequentialStream *chp = NULL;

int main(void) {
  halInit();
  chSysInit();

  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(500);


  chThdSleepMilliseconds(2000);
  chp = (BaseSequentialStream*)&SDU1;
  
  palSetPadMode(GPIOA, 1,
  		PAL_MODE_OUTPUT_PUSHPULL |
  		PAL_STM32_OSPEED_HIGHEST);
  palWritePad(GPIOA, 7, 0);

  palSetPadMode(GPIOA, 0,
		PAL_MODE_INPUT_PULLDOWN |
		PAL_STM32_OSPEED_HIGHEST);

  chprintf(chp, "Example responder starting up\n");

  while(true) {

    uint32_t value = 0;

    value = palReadPad(GPIOA, 0);
    
    palWritePad(GPIOA,1, value);
  }

  return 0; //unreachable
}
