#include <avr/io.h>
#include "spi.h"

// spiXfer() takes a byte and sends it out via the USI function. It does
//   NOT handle the chip select; user must do that before calling spiXfer().
//   It returns the value that was shifted in.

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