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

static mailbox_t mb;
static msg_t b[100];

static void send_message(uint32_t msg) {
  msg_t msg_val;
  msg_val = chMBPostI(&mb, msg);
  if (msg_val != MSG_OK) {
    /* error send message */
  }
}


static bool read_message_block(uint32_t *msg) {

  msg_t msg_value;
  bool ret = false;

  int r = chMBFetchTimeout(&mb, &msg_value, TIME_INFINITE);

  if (r == MSG_OK ) {
    *msg = msg_value;
    r = true;
  } 
  return r;
}



BaseSequentialStream *chp = NULL;

static void button_cb(void *arg) {

  chSysLockFromISR();
  
  uint32_t state = palReadPad(GPIOA, 0);

  send_message(state);
  
  chSysUnlockFromISR();
}





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

  palEnablePadEvent(GPIOA, 0, PAL_EVENT_MODE_BOTH_EDGES);
  palSetPadCallback(GPIOA, 0, button_cb, NULL);
   
  chMBObjectInit(&mb, b, 100);

  
  chprintf(chp, "Example responder starting up\n");

  uint32_t value = 0;
  while(true) {

    if (read_message_block(&value)) {
      palWritePad(GPIOA,1, value);
    }
  }

  return 0; //unreachable
}
