/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

spi.cpp
Functional support for the SPI. This doesn't seem to be well supported in the
AVR libraries, but maybe I didn't look in the right places.
******************************************************************************/

#include <avr/io.h>
#include "spi.h"

// spiXfer() takes a byte and sends it out via the USI function. It does
//   NOT handle the chip select; user must do that before calling spiXfer().
//   It returns the value that was shifted in. Please review the datasheet
//   for an explanation of the hows and whys- it's important to note that
//   unlike more advanced processors, the Tinty2313a does not support a
//   hands-off shift method. The data must be clocked out under software
//   control!
uint8_t spiXfer(uint8_t data)
{
	USIDR = data;
	while ((USISR & (1<<USIOIF)) == 0)
	{
		USICR = (0<<USIWM1) | (1<<USIWM0) | (1<<USICS1) | (0<<USICS0) | (1<<USICLK) | (1<<USITC);
	}
	USISR = (1<<USIOIF);
	return USIDR;
}