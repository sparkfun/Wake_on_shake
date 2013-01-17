/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

serial.h
Function definitions for the ADXL362.
******************************************************************************/

#ifndef _serial_h_included
#define _serial_h_included

void serialWriteChar(char);			// Single character write.
void serialWrite(char*);			// String constant write. Terminates with a
									//  a CR and an LF for terminal happiness.
									//  Blocking.
void serialWriteInt(unsigned int);  // Convert a 16-bit unsigned value into
									//  ASCII characters and send it out.
									//  Terminates with CR and LF. Blocking.
void serialNewline(void);
									
#endif
