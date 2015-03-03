/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

serial.cpp
Function implementation for serial write handling.
******************************************************************************/

#include <avr/io.h>
#include <stdio.h>
#include "serial.h"

// Print a single character out to the serial port. Blocks until write has
//   completed- is that a mistake?
void serialWriteChar(char data)
{
	UDR = data;
	while ((UCSRA & (1<<TXC))==0){}   // Wait for the transmit to finish.
	UCSRA |= (1<<TXC);				// Clear the "transmit complete" flag.
}

// serialWrite() takes a pointer to a string and iterates over that string
//   until it finds the C end-of-string character ('\0'). It's a blocking
//   operation and does not return until the print operation is completed.
void serialWrite(char* data)
{
	do
	{
		serialWriteChar((char)*data);   // Print the first character.
		data++;							// Increment the pointer.
	} while (*data != '\0');			// Check for the end of the string.
	serialNewline();
}

// Convert a 16-bit unsigned value into ASCII characters and dump it out
//   to the serial port.
void serialWriteInt(unsigned int data)
{	
	uint8_t tenth = 0;
	uint8_t thou = 0;
	uint8_t huns = 0;
	uint8_t tens = 0;
	uint8_t ones = 0;
	// This is an awkward shifting/dividing method of isolating the individual
	//   digits of the number. I'm sure there's a better way, but done is.
	tenth = (data/10000);
	data -= tenth*10000;
	thou = (uint8_t)(data/1000);
	data -= (uint16_t)thou*1000;
	huns = (uint8_t)(data/100);
	data -= (uint16_t)huns*100;
	tens = (uint8_t)(data/10);
	ones = (uint8_t)(data%10);
	
	tenth += 48;
	thou += 48;
	huns += 48;
	tens += 48;
	ones += 48;
	// Write the individual digits out, followed by a line feed and CR.
	serialWriteChar(tenth);
	serialWriteChar(thou);
	serialWriteChar(huns);
	serialWriteChar(tens);
	serialWriteChar(ones);
	serialNewline();
}

void serialNewline(void)
{
	serialWriteChar((char)'\n');
	serialWriteChar((char)'\r');
}
