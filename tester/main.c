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

#include "timer.h"
#include "ISNS20.h"

BaseSequentialStream *chp = NULL;

char input_buf[1024];

int inputline(BaseSequentialStream *chp, char *buffer, int size) {
  int n = 0;
  unsigned char c;
  for (n = 0; n < size - 1; n++) {

    c = streamGet(chp);
    switch (c) {
    case 127: /* fall through to below */
    case '\b': /* backspace character received */
      if (n > 0)
        n--;
      buffer[n] = 0;
      //streamPut(chp,0x8); /* output backspace character */
      //streamPut(chp,' ');
      //streamPut(chp,0x8);
      n--; /* set up next iteration to deal with preceding char location */
      break;
    case '\n': /* fall through to \r */
    case '\r':
      buffer[n] = 0;
      return n;
    default:
      if (isprint(c)) { /* ignore non-printable characters */
        //streamPut(chp,c);
        buffer[n] = c;
      } else {
        n -= 1;
      }
      break;
    }
  }
  buffer[size - 1] = 0;
  return 0; // Filled up buffer without reading a linebreak
}

static THD_WORKING_AREA(mailmanArea, 1024);

static THD_FUNCTION(mailman, arg) {

  (void)arg;

  char s_str[256];
  
  while (true) {
    timer_msg_t msg;

    if (block_mail(&msg) ) {

      chprintf(chp,"You got mail!\r\n");
      chprintf(chp,"start: %u\r\n", msg.start);
      chprintf(chp,"stop:  %u\r\n", msg.stop);
      chprintf(chp,"diff ticks: %u\r\n", msg.stop - msg.start);
      double ticks = msg.stop - msg.start;
      double sec  = ticks / 84000000.0; /* ticks per second */
      snprintf(s_str,256, "%f s", sec);
      chprintf(chp,"time: %s\r\n", s_str);
      timer_reset(); 
    } else {
      /* Error: something wrong with messaging */
      //chprintf(chp, "no message\r\n");
    }
  }
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

  /* Start up the mailman */
  chprintf(chp,"Starting up the mailman\r\n");
  (void)chThdCreateStatic(mailmanArea,
			  sizeof(mailmanArea),
			  NORMALPRIO,
			  mailman, NULL); 


  start_spi_thread();
  
  palSetPadMode(GPIOA, 7,
  		PAL_MODE_OUTPUT_PUSHPULL |
  		PAL_STM32_OSPEED_HIGHEST);
  palWritePad(GPIOA, 7, 0);

  palSetPadMode(GPIOA, 0,
		PAL_MODE_INPUT_PULLDOWN |
		PAL_MODE_ALTERNATE(2));
  palSetPadMode(GPIOA, 1,
		PAL_MODE_INPUT_PULLDOWN |
		PAL_MODE_ALTERNATE(2));

  palSetPadMode(GPIOA, 2,
   		PAL_MODE_OUTPUT_PUSHPULL |
   		PAL_STM32_OSPEED_HIGHEST);

  chprintf(chp, "Tester starting up\n");

  timer_init();

  while(true) {

    inputline(chp, input_buf, 256); /* blocks */

    if (strncmp(input_buf, "init", 4) == 0) {
      chprintf(chp, "OK!\n");
    } else if (strncmp(input_buf, "SET", 3) == 0) {
      /* Proof of concept, something smarter later */
      if (strncmp(input_buf+4, "GPIOA1", 6) == 0) {
    	palWritePad(GPIOA, 7, 1);
      }
    } else if (strncmp(input_buf, "CLR", 3) == 0) {
      /* Proof of concept, something smarter later */
      if (strncmp(input_buf+4, "GPIOA1", 6) == 0) {
    	palWritePad(GPIOA, 7, 0);
      }
    }
    memset(input_buf,0,1024);
  }

  return 0; //unreachable
}
