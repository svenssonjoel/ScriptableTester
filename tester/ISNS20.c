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

#include "ISNS20.h"

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

extern BaseSequentialStream *chp;

const SPIConfig spicfg = {
  false,
  NULL,
  GPIOB,
  SPI_ISNS20_CS_PIN,
  SPI_CR1_LSBFIRST | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0 | SPI_CR1_CPOL | SPI_CR1_RXONLY,
  0
};


void init_spi(void) {

  /* Configure GPIOs */
  palSetPadMode(SPI_GPIO, SPI_CK_PIN,
		PAL_MODE_ALTERNATE(6) |
		PAL_STM32_OSPEED_HIGHEST);
  
  palSetPadMode(SPI_GPIO, SPI_MISO_PIN,
		PAL_MODE_ALTERNATE(6) |
		PAL_STM32_OSPEED_HIGHEST);
  
  palSetPadMode(SPI_GPIO, SPI_MOSI_PIN,
   		PAL_MODE_ALTERNATE(6) |
   		PAL_STM32_OSPEED_HIGHEST);

  palSetPadMode(SPI_GPIO, SPI_ISNS20_CS_PIN,
		PAL_MODE_OUTPUT_PUSHPULL |
		PAL_STM32_OSPEED_HIGHEST);

  
} 

static uint8_t rx_buf[2];

double read_sample(void) {

  rx_buf[0] = 0;
  rx_buf[1] = 0;
  
  spiAcquireBus(&SPID3);
  spiStart(&SPID3, &spicfg);
  
  spiSelect(&SPID3);
  spiReceive(&SPID3, 2, rx_buf);
  spiUnselect(&SPID3);
  spiReleaseBus(&SPID3);

  /* uint32_t res = 0; */
  /* res |= rx_buf[0]; */
  /* res <<= 16; */
  /* res |= rx_buf[1]; */

  chprintf(chp, "RAW: %x | %x \r\n", rx_buf[0], rx_buf[1]);
  
  //double r = res;
  //r = (r / 4096.0 * (-3.0)) / 0.066; /*V per A ???? */
  return 0.0;
}


static THD_WORKING_AREA(spiArea, 1024);

static THD_FUNCTION(spiThread, arg) {

  (void) arg;

  char s_str[256];

  while(true) {

    double s = read_sample();

    // snprintf(s_str,256, "%f A", s);
    //chprintf(chp, "Sample: %s \r\n", s_str); 
    
    chThdSleepMilliseconds(250);
  }
}

void start_spi_thread(void) {
  init_spi();
  
  (void)chThdCreateStatic(spiArea,
			  sizeof(spiArea),
			  NORMALPRIO,
			  spiThread, NULL); 
}

					 
