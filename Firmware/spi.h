/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

spi.h
Function definitions for SPI mode using the USI peripheral.
******************************************************************************/

#ifndef _spi_h_included
#define _spi_h_included

uint8_t spiXfer(uint8_t);	// 8-bit data transfer function using the onboard
							//   USI peripheral.

#endif