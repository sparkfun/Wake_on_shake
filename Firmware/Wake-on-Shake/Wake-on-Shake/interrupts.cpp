#include <avr/io.h>
#include <avr/interrupt.h>
#include "interrupts.h"
#include "wake-on-shake.h"
#include "serial.h"
#include "eeprom.h"

// Timer1 overflow ISR
ISR(TIMER1_OVF_vect)
{
	sleepyTime = true;
}

ISR(INT0_vect)
{
	TCNT1 = t1Offset;
	GIMSK = (0<<INT0);
}

ISR(INT1_vect)
{
	return;
}

ISR(USART_RX_vect)
{
	TCNT1 = t1Offset;
	unsigned char data = UDR;
	serialWriteChar(data);
	if (((data == '\n') | (data == '\r')) & (mode == ' '));
	else if (((data == '\n') | (data == '\r')) & (mode != ' '))
	{
		serialWriteInt(inputBufferValue);
		serialWriteChar(mode);
		switch(mode)
		{
			case 'x':
			//EEPROMWriteWord((uint8_t)X_THRESH, inputBufferValue);
			x_threshold = inputBufferValue;
			break;
			case 'y':
			//EEPROMWriteWord((uint8_t)Y_THRESH, inputBufferValue);
			y_threshold = inputBufferValue;
			break;
			case 'z':
			//EEPROMWriteWord((uint8_t)Z_THRESH, inputBufferValue);
			z_threshold = inputBufferValue;
			break;
			case 'd':
			inputBufferValue = 65535 - inputBufferValue;
			//EEPROMWriteWord((uint8_t)WAKE_OFFS, inputBufferValue);
			t1Offset = inputBufferValue;
			break;
		}
		inputBufferValue = 0;
		mode = ' ';
	}
	if (mode == ' ')
	{
		mode = data;
	}
	else
	{
		inputBufferValue *= 10;
		inputBufferValue += (data - 48);
	}
}
