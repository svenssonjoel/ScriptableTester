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

static THD_WORKING_AREA(responseTestArea, 1024);

bool response_test_running = false;


static volatile uint32_t num_tests = 1000;
static volatile uint32_t response_timeout = 1000;

#define RESPONSE_TEST_RUNNING 2
#define RESPONSE_TEST_IDLE    0
#define RESPONSE_TEST_INIT    1


/* TODO: Maybe split up TEST_RUNNING state into two 
   separate states for test preparation and test performing */
static THD_FUNCTION(response_tester, arg) {
  (void)arg;
  
  char s_str[256];
  timer_msg_t msg;
  timer_msg_t nonsense;
  uint32_t nTests = num_tests;
  uint32_t timeout = response_timeout;
  
  uint32_t state = RESPONSE_TEST_IDLE;
  
  while (true) {
    switch(state) {
    case RESPONSE_TEST_IDLE:
      
      if (response_test_running) {
	state = RESPONSE_TEST_INIT;
      } else {
	chThdSleepMilliseconds(250);
      }
      break;
    case RESPONSE_TEST_INIT:
      
      nTests = num_tests;
      timeout = response_timeout;
      state = RESPONSE_TEST_RUNNING;
       
      break;
    case RESPONSE_TEST_RUNNING:
      if (nTests == 0) {
	response_test_running = false;
	state = RESPONSE_TEST_IDLE;
	/* Test is finished */
	chprintf(chp,"#RESPONSE_TEST_DONE\r\n");
      } else if (response_test_running) {

	/* Set up for a test */
	palWritePad(GPIOA, 2, 0);
	
	/* wait for GPIOA 0 and GPIOA 1 to both read 0. */
	uint32_t n = 0;
	while (palReadPad(GPIOA, 0) || palReadPad(GPIOA, 1)) {
	  chThdSleepMilliseconds(1);
	  if (n++ >= response_timeout)
	    break;	       
	}
	if (palReadPad(GPIOA, 0) || palReadPad(GPIOA, 1)) {
	  state = 1777; /* Error state: SUT never lowered response pin
	                   or connection between GPIO A2 and GPIO A0 
                           came loose. */
	  break;
	}
	
	timer_reset();

	/* take a short break */
	chThdSleepMilliseconds(15);
	
	nTests --;
	
	/* clear mailbox */
	while (poll_mail(&nonsense));

	/* Initiate a response test by writing a one to GPIOA 2 */
	palWritePad(GPIOA, 2, 1);
	
	if (block_mail(&msg, timeout) ) {
	  double ticks = msg.stop - msg.start;

	  // chprintf(chp," start: %u\n", msg.start);
	  //chprintf(chp," stop:  %u\n", msg.stop);
	  //chprintf(chp," ticks: %u\n", msg.stop - msg.start);
	  double sec  = ticks / 84000.0; /* ticks per millisecond */
	  snprintf(s_str,256, "%lf", sec);

	  if ( msg.start >= msg.stop ||
	       ticks > (timeout * 84000) ) {
	    chprintf(chp, "#REPONSE_MALFORMED\r\n");
	    /* Getting a large amount of malformed responses 
	       should be a warning flag to the user */
	  } else {	  
	    chprintf(chp,"#RESPONSE_LATENCY: %s\r\n", s_str);
	  }
	  
	  /* We had a response, so prepare for the next test */
	} else {
	  state = 1323; 
	}
      }
      break;
    default :
      chprintf(chp,"#RESPONSE_TEST_ERROR %d\r\n", state);
      state = RESPONSE_TEST_IDLE;
      response_test_running = false;
      break; /* end the tests */
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
  
  chprintf(chp,"Starting up response time tester\r\n");
  (void)chThdCreateStatic(responseTestArea,
			  sizeof(responseTestArea),
			  NORMALPRIO,
			  response_tester, NULL); 


  //start_spi_thread();

  /* TODO Start naming these GPIOs so we know what is what */

  /* GPIOA 7 is a generic "stimuli GPIO" used to simulate for 
     example a button press on the system under test */ 
  palSetPadMode(GPIOA, 7,
  		PAL_MODE_OUTPUT_PUSHPULL |
  		PAL_STM32_OSPEED_HIGHEST);
  palWritePad(GPIOA, 7, 0);

  /* TODO: Maybe move these out to the "timer.c/timer.h"  */ 
  /* Grab timer to CCR[0] on rising edge on GPIOA 0 */ 
  palSetPadMode(GPIOA, 0,
		PAL_MODE_INPUT_PULLDOWN |
		PAL_MODE_ALTERNATE(2));

  /* Grab timer to CCR[1] on rising edge on GPIOA 1 */ 
  palSetPadMode(GPIOA, 1,
		PAL_MODE_INPUT_PULLDOWN |
		PAL_MODE_ALTERNATE(2));

  /* Stimuli GPIO, connect to GPIOA0 on "tester" and to the SUT */
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
    } else if (strncmp(input_buf, "RSPTST", 6) == 0) {
      /* Start a response time test (if it is not already running) */
      if (!response_test_running) {
	char *cmd = strtok(input_buf, " ");
	char *s_arg0 = strtok(NULL, " ");
	char *s_arg1 = strtok(NULL, " ");
	char *s_null = strtok(NULL, " ");

	(void) cmd;
	(void) s_null;
	
	if (!s_arg0 || !s_arg1) {
	  chprintf(chp, "RSPTST incorrect arguments\r\n");
	} else {

	  num_tests = (uint32_t)atoi(s_arg0);
	  response_timeout   = (uint32_t)atoi(s_arg1);
	  response_test_running = true;
	}
      } else {
	chprintf(chp, "RSPTST already running\r\n");
      }
    } else {
      chprintf(chp, "Unknown command: %s\r\n", input_buf);
    }
    
    memset(input_buf,0,1024);
  }

  return 0; //unreachable
}
