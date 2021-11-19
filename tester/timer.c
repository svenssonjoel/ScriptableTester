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

#include "ch.h"
#include "hal.h"
#include "timer.h"

static stm32_tim_t *tim5 = NULL;


#define MAX_MESSAGES 64

msg_t box_contents[MAX_MESSAGES];
MAILBOX_DECL(mb, box_contents, MAX_MESSAGES);

timer_msg_t msg_storage[MAX_MESSAGES];
MEMORYPOOL_DECL(msg_pool, MAX_MESSAGES, PORT_NATURAL_ALIGN, NULL);


static bool send_mail(timer_msg_t t) { 
  bool r = true; /*success*/
  timer_msg_t *m = (timer_msg_t*)chPoolAllocI(&msg_pool);
  
  if (m) { 
    *m = t;
    
    msg_t msg_val = chMBPostI(&mb, (msg_t)m);
    if (msg_val != MSG_OK) {  /* failed to send */
      chPoolFree(&msg_pool,(void*) m);
      r = false;
    }
  }
 
  return r;
}

bool poll_mail(timer_msg_t *t) {

  bool r = true;
  msg_t msg_val;

  int m = chMBFetchTimeout(&mb, &msg_val, TIME_IMMEDIATE);

  if (m == MSG_OK) {
    *t = *(timer_msg_t*)msg_val;

    chPoolFree(&msg_pool, (void*)msg_val); /* free the pool allocated pointer */
  } else {
    r = false;
  }
  return r;
}

bool block_mail(timer_msg_t *t, uint32_t timeout) {

  bool r = true;
  msg_t msg_val;

  int m = chMBFetchTimeout(&mb, &msg_val, chTimeMS2I(timeout));

  if (m == MSG_OK) {
    *t = *(timer_msg_t*)msg_val;

    chPoolFree(&msg_pool, (void *)msg_val); /* free the pool allocated pointer */
  } else {
    r = false;
  }
  return r;
}




void timer_init(void) {
  

  /* Initialize the memory pool for tick messages */
  chPoolLoadArray(&msg_pool, msg_storage, MAX_MESSAGES);
  /* initialize mailbox */
  chMBObjectInit(&mb, box_contents, MAX_MESSAGES);

  rccEnableTIM5(true);
  rccResetTIM5();
  
  nvicEnableVector(STM32_TIM5_NUMBER, 7); // STM32_IRQ_TIM5_PRIORITY);

  
  tim5 = STM32_TIM5;  /* gives direct access to the tim5 registers */

  /*
    typedef struct {
    volatile uint32_t     CR1;      - Control register 1
    volatile uint32_t     CR2;      - Control register 2
    volatile uint32_t     SMCR;     - Slave mode control register
    volatile uint32_t     DIER;     - DMA/Interrupt enable register
    volatile uint32_t     SR;       - Status register
    volatile uint32_t     EGR;      - Event generation register
    volatile uint32_t     CCMR1;    - Capture/Compare mode register 1
    volatile uint32_t     CCMR2;    - Capture/Compare mode register 2
    volatile uint32_t     CCER;     - Capture/Compare enable register
    volatile uint32_t     CNT;      - Count register.
    volatile uint32_t     PSC;      - Prescaler (1 - 65535).
    volatile uint32_t     ARR;      - Auto reload register.
    volatile uint32_t     RCR;      
    volatile uint32_t     CCR[4];   - Compare/Capture registers.
    volatile uint32_t     BDTR;
    volatile uint32_t     DCR;      - DMA control register
    volatile uint32_t     DMAR;     - DMA Address for full transfer
    volatile uint32_t     OR;       - Option register.
    volatile uint32_t     CCMR3;    - Capture/compare mode register 3 
    volatile uint32_t     CCXR[2];
    } stm32_tim_t;
  */

  tim5->PSC = 0x0;     // counter rate is input_clock / (0x0+1)
  tim5->ARR = 0xFFFFFFFF; // Value when counter should flip to zero.

  tim5->CCR[0] = 0x0; /* clear Capture values */ 
  tim5->CCR[1] = 0x0;
  tim5->CCR[2] = 0x0;
  tim5->CCR[3] = 0x0; 

  tim5->CCMR1 = 0x0;
  //tim5->CCMR1 |= 0xF000F000; /* filter sample rate */
  //tim5->CCMR1 &= 0xFFFFFFFC; /* Clear two bits */
  tim5->CCMR1 |= 0x00000001; /* CC1S = 01, input IC1 -> TI1 */
  //tim5->CCMR1 &= 0xFFFFFCFF; /* Clear two bits */
  tim5->CCMR1 |= 0x00000100; /* CC2S = 01, input IC2 -> TI2 */

  tim5->CCER &= 0xFFFFFFF2; /* clear some */
  tim5->CCER |= 0x00000001; /* activate capture on channel 1 */
  tim5->CCER &= 0xFFFFFF2F; /* clear some */
  tim5->CCER |= 0x00000010; /* activate capture on channel 2 */ 

  //tim5->OR &= 0xFFFFFF9F;
  
  tim5->DIER |= 0x4; 
  
  tim5->CNT = 0;
  tim5->EGR = 0x1; // Update event (Makes all the configurations stick)
  tim5->CR1 |= 0x1; // enable
  
}

void timer_reset(void) {

  //tim5->CR1 &= ~0x1; /* Disable counter */
  //tim5->CNT = 0x0;   /* Clear count */
  tim5->CCR[0] = 0;
  tim5->CCR[1] = 0;
  //tim5->EGR = 0x1;   /* Maybe not needed */ 
  //tim5->CR1 |= 0x1;  /* Start timer */
}

uint32_t pin_state = 0;

OSAL_IRQ_HANDLER(STM32_TIM5_HANDLER) {
  OSAL_IRQ_PROLOGUE();
  
  uint32_t sr = tim5->SR;
  sr &= tim5->DIER & STM32_TIM_DIER_IRQ_MASK;
  tim5->SR = ~sr;  

 
  timer_msg_t msg;
  msg.start = tim5->CCR[0];
  msg.stop  = tim5->CCR[1];
  
  osalSysLockFromISR();
  send_mail(msg);
  osalSysUnlockFromISR();
  
  OSAL_IRQ_EPILOGUE();
}


