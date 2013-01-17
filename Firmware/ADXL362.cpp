/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

ADXL362.cpp
Function code for the ADXL362. If you're looking for register aliases,
check out xl362.h, which was provided by Analog Devices.
******************************************************************************/

#include <avr/io.h>
#include "ADXL362.h"
#include "spi.h"
#include "xl362.h"
#include "eeprom.h"
#include "wake-on-shake.h"

// ADXLConfig() sets all the necessary registers on the ADXL362 up to support
//   the wake-on-shake type application.
void ADXLConfig(void)
{
	// Activity threshold level (0x20)-
	//   Defaults to 150mg; user can change this.
	ADXLWriteByte((uint8_t)XL362_THRESH_ACTH, EEPROMReadByte(ATHRESH));
	ADXLWriteByte((uint8_t)XL362_THRESH_ACTL, EEPROMReadByte(ATHRESH + 1));
	// Inactivity threshold level (0x23)-
	//   Written to 50 to give a 50mg sleep detection level
	ADXLWriteByte((uint8_t)XL362_THRESH_INACTH, EEPROMReadByte(ITHRESH));
	ADXLWriteByte((uint8_t)XL362_THRESH_INACTL, EEPROMReadByte(ITHRESH+1));
	// Inactivity timer (0x25)-
	//   Written to 15; wait 15 samples (~2.5 seconds) before going back
	//   to sleep.
	ADXLWriteByte((uint8_t)XL362_TIME_INACTH, EEPROMReadByte(ITIME));
	ADXLWriteByte((uint8_t)XL362_TIME_INACTL, EEPROMReadByte(ITIME+1));	
	// Activity/Inactivity control register (0x27)-
	//   Needs to be set to LOOP mode (5:4 = 11)
	//   We want referenced measurement mode for inactivity (3 = 1)
	//   We need to activate inactivity detection (2 = 1)
	//   We want referenced measurement mode for activity (1 = 1)
	//   We need to activate activity detection (0 = 1)
	ADXLWriteByte((uint8_t)XL362_ACT_INACT_CTL, (uint8_t)0xFF);
	// INT1 function map register (0x2A)-
	//   Needs to be set to "Active Low" (7 = 1)
	//   Needs to be set to activity mode (4 = 1)
	//   Other bits must be zero.
	ADXLWriteByte((uint8_t)XL362_INTMAP1, (uint8_t)0b10010000);
	// Power control register (0x2D)-
	//   Needs to be set to wake mode (3 = 1)
	//   Need to turn on sampling mode (1:0 = 10)
	//   Other bits must be zero
	ADXLWriteByte((uint8_t)XL362_POWER_CTL, (uint8_t)0x0A);
}

// Simple functions to assert chip select and copy data in and out of the
//   ADXL362. 
uint8_t ADXLReadByte(uint8_t addr)
{
	PORTB &= !(1<<PB4);	
	spiXfer((uint8_t)XL362_REG_READ);
	spiXfer(addr);
	addr = spiXfer(addr);
	PORTB |= (1<<PB4);
	return addr;
}

void ADXLWriteByte(uint8_t addr, uint8_t data)
{
	PORTB &= !(1<<PB4);
	spiXfer((uint8_t)XL362_REG_WRITE);
	spiXfer(addr);
	spiXfer(data);
	PORTB |= (1<<PB4);
}