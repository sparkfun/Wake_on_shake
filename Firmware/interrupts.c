/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

interrupts.cpp
Interrupt service routine code. ISR length is minimized as much as possible;
the application in question doesn't really have hard real-time deadlines, so
we're free to be kind of lax with our response times.
******************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "interrupts.h"
#include "wake-on-shake.h"
#include "serial.h"
#include "eeprom.h"

extern uint16_t				t1Offset;		// See Wake-on-Shake.cpp
extern volatile uint8_t		sleepyTime;		// See Wake-on-Shake.cpp
extern volatile uint8_t     serialRxData;	// See Wake-on-Shake.cpp

// Timer1 overflow ISR- this is the means by which the device goes to sleep
//   after it's been on for a certain time. Timer1 has been set up to tick
//   on clock/1024, which is ~1ms ticks; it's a 16-bit overflow, so left to
//   it's own devices, it will overflow every 65536 ticks, or after a bit
//   more than a minute. To shorten that time, we prime TCNT1
ISR(TIMER1_OVF_vect)
{
	sleepyTime = TRUE;
}

// INT0 ISR- This is one way the processor can wake from sleep. INT0 is tied
//   externally to the RX pin, so traffic on the serial receive line will
//   wake up the part when it is asleep. Note that the receive interrupt
//   can't wake the processor from sleep- don't try!
ISR(INT0_vect)
{
	TCNT1 = t1Offset;				// Reset our counter for on-time.
	sleepyTime = FALSE;				// Indicate wakefulness to main loop.
	GIMSK = (0<<INT0)|(0<<INT1);	// Disable INT pins while we're awake.
									//  This is important b/c the INT pins
									//  cause an interrupt on LOW rather
									//  than on an edge, so the interrupt
									//  will continue to fire as long as
									//  the pin is low unless it is disabled.
}

// INT1 ISR- this is the primary way the processor wakes from sleep. INT1 is
//   tied to the interrupt output pin on the ADXL362, which goes low when
//   motion is detected.
ISR(INT1_vect)
{
	TCNT1 = t1Offset;				// See INT0 ISR for details.
	sleepyTime = FALSE;
	GIMSK = (0<<INT0)|(0<<INT1); 
}

// USART_RX ISR- gets called when the processor is awake and a complete
//   byte (including stop bit) has been received by the USART. This
//   interrupt CANNOT be used to wake the processor, so don't try it.
ISR(USART_RX_vect)
{
	TCNT1 = t1Offset;	// Reset the wakefulness timer, so the processor
						//   doesn't go to sleep while the user is
						//   interacting with it.
	serialRxData = UDR; // Pass the data back to the main loop for parsing.
}
