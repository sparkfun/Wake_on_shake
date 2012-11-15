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
	TCNT1 = 55000;
	sleepyTime = false;
	GIMSK = (0<<INT0);
}

ISR(INT1_vect)
{
	return;
}

ISR(USART_RX_vect)
{
	TCNT1 = 55000;
	serialRxData = UDR;
}
