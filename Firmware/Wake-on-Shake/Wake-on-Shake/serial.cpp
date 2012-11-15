#include <avr/io.h>
#include "serial.h"

void serialWriteChar(char data)
{
	UDR = data;
	while ((UCSRA & (1<<TXC))==0){}   // Wait for the transmit to finish.
	UCSRA |= (1<<TXC);				// Clear the "transmit complete" flag.
}

// serialWrite() takes a pointer to a string and iterates over that string
//   until it finds the C end-of-string character ('\0'). It's a blocking
//   operation and does not return until the print operation is completed.
void serialWrite(const char* data)
{
	do
	{
		serialWriteChar((char)*data);
		data++;							// Increment the pointer.
	} while (*data != '\0');			// Check for the end of the string.
	serialWriteChar((char)'\n');
	serialWriteChar((char)'\r');
}

void serialWriteInt(unsigned int data)
{
	uint8_t tenth = 0;
	uint8_t thou = 0;
	uint8_t huns = 0;
	uint8_t tens = 0;
	unsigned char ones = 0;
	tenth = (data/10000);
	data -= tenth*10000;
	thou = (unsigned char)(data/1000);
	data -= (unsigned int)thou*1000;
	huns = (unsigned char)(data/100);
	data -= (unsigned int)huns*100;
	tens = (unsigned char)(data/10);
	ones = (unsigned char)(data%10);
	tenth += 48;
	thou += 48;
	huns += 48;
	tens += 48;
	ones += 48;
	
	serialWriteChar(tenth);
	serialWriteChar(thou);
	serialWriteChar(huns);
	serialWriteChar(tens);
	serialWriteChar(ones);
}



