/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

ADXL362.h
Function definitions for the ADXL362. If you're looking for register aliases,
check out xl362.h, which was provided by Analog Devices.
******************************************************************************/

#ifndef _ADXL326_h_included
#define _ADXL326_h_included

uint8_t ADXLReadByte(uint8_t);				// Does all the work to read a byte
											//   from one of the ADXL362's
											//   internal registers.
void    ADXLWriteByte(uint8_t, uint8_t);	// Does all the work to write a byte
											//   to one of the ADXL362's
											//   internal registers.
void    ADXLConfig(void);					// Set up all the necessary values
											//   to put the ADXL362 into the
											//   mode we need for this product,
											//   including user set variables.
											
/* NB- FIFO reading is not supported by the code at this time! It wasn't needed
      for this code, and so was not implemented.
*/

#endif