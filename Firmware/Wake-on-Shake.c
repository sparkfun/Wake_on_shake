/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

Original wake-on-shake concept by Nitzan Gadish, Analog Devices

Wake-on-Shake is a simple but powerful board designed to power up a larger
circuit when it detects motion. The load has the option of holding its power
even after the Wake-on-Shake releases it, so the load circuit can complete its
"mission".

It also provides a very simple UI which can be used to set the time before
shutdown and wake-up sensitivity. There are other, more powerful features
available, but most users will not need them.

******************************************************************************/
// avrdude programming string
//avrdude -p t2313 -B 10 -P usb -c avrispmkii -U flash:w:Wake-on-Shake.hex -U hfuse:w:0xdf:m -U lfuse:w:0x64:m

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "eeprom.h"
#include "wake-on-shake.h"
#include "interrupts.h"
#include "spi.h"
#include "ADXL362.h"
#include "xl362.h"
#include "ui.h"

uint16_t			t1Offset;			// This value, when written to TCNT1, 
										//   is the offset to the delay before
										//   sleep. It is 65535 - (delay)ms.
volatile uint8_t	sleepyTime = FALSE; // Flag used to communicate from the
										//   ISR to the main program to send
										//   the device into sleep mode.
volatile uint8_t    serialRxData = 0;	// Data passing variable to get data
										//   from the receive ISR back to main.
										
// main(). If you don't know what this is, you need to do some serious
//  work on your fundamentals.

int main(void)
{
	// First, let's configure the directions (in/out) of the various pins. Writing a
	//   '1' to the appropriate bit in a DDRx register will render that pin an
	//   output.
	
	// Port A- both pins are unused, so we'll make them both outputs and drive them
	//   low to save power.
	DDRA = (1<<PA1) | (1<<PA0);
	// Port B- PB0-3 are unused; make them outputs and tie them low. PB4 is !CS
	//   for the ADXL362, so it should be an output. PB5 is MISO from the ADXL362, so
	//   leave it an input. PB6 is MOSI to ADXL362, and PB7 is SCK, so they should be
	//   outputs.
	DDRB = (1<<PB7) | (1<<PB6) | (1<<PB4) | (1<<PB3) | (1<<PB2)| (1<<PB1) | (1<<PB0);
	// No port C pins on this chip
	// Port D- PD0 is the serial receive input. PD1 is serial transmit output. PD2 is
	//   is an external interrupt used to wake the processor on serial activity. PD3
	//   is the interrupt from the ADXL362, used to wake the device on detected
	//   motion. PD4 is the MOSFET turn-on signal, so should be an output. PD5 and
	//   PD6 are unused; make them outputs and tie them low. PD7 doesn't exist.
	DDRD = (1<<PD6) | (1<<PD5) | (1<<PD4) | (1<<PD1);
	
	// Now let's configure some initial IO states
	
	// Port A- both set low to reduce current consumption
	PORTA = (0<<PA1) | (0<<PA0);
	// Port B- PB0-3 should be low for power consumption; PB4 is !CS, so bring it
	//   high to keep the ADXL362 non-selected. Others are don't care.
	PORTB = (1<<PB4) | (0<<PB3) | (0<<PB2) | (0<<PB1) | (0<<PB0);
	// Port C doesn't exist
	// Port D- PD5 and PD6 should be tied low; others are (for now) don't care.
	//   Also, PD2/PD0 should be made high to enable pullup resistor for when no
	//   serial connection is present.
	PORTD = (1<<PD6) | (1<<PD5) | (1<<PD2) | (1<<PD0);
	
	// Interrupt configuration is next. We'll need two interrupts: INT0 and INT1.
	//   INT0 will wiggle when a serial connection occurs. INT1 will wiggle when
	//   the ADXL362 detects motion and wakes up.
	
	// MCUCR- Bits 0:3 control the interrupt event we want to be sensitive to.
	//   We'll set it so a low level is what each pin is looking for.
	MCUCR = (0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
	// GIMSK- These are mask bits for the pin change interrupts. Writing a '1'
	//   enables the appropriate interrupt.
	GIMSK = (0<<INT1) | (0<<INT0);
	
	// Now, set up the USI peripheral for communication with the ADXL362 part.
	//   The ADXL362 uses SPI Mode 0- CPHA = CPOL = 0.
	
	// UISCR- USWM1:0 are mode select pins; 01 is three-wire mode.
	//   USICS1:0 are clock source select pins; 10 is software control.
	//   USICLK puts the 4-bit data counter under software control.
	//   Strobing USITC toggles the 4-bit clock signal.
	USICR = (0<<USIWM1) | (1<<USIWM0) | (1<<USICS1) | (0<<USICS0) | (1<<USICLK);
	// USISR- Writing '1' to USIOIF will clear the 4-bit counter overflow
	//   flag and ready it for the next transfer. Implicit here is a write
	//   of zeroes to bits 3:0 of this register, which also clears the 4-bit
	//   counter.
	USISR = (1<<USIOIF);
	
	// Set up the USART peripheral for writing in and out. This isn't used
	//   during normal operation- only during user configuration. We'll
	//   set the mode to 9600 baud, 8-N-1. Pretty standard, really.

	// For 9600 baud, at 1.000MHz (which is our clock speed, since we're
	//   using the internal oscillator clocked down), UBRR should be set to
	//   12, and the U2X bit of UCSRA should be set to '1'.
	UBRRH = 0;
	UBRRL = 12;
	UCSRA = (1<<U2X);
	// UCSRB- RXEN and TXEN enable the transmit and receive circuitry.
	//   UCSZ2 is a frame size bit; when set to 0 (as here), the size is
	//   determined by UCSZ1:0 in UCSRC. RXCIE is the receive interrupt
	//   enable bit; we want that interrupt for handling incoming settings
	//   changes while the part is awake.
	UCSRB = (1<<RXCIE) | (1<<RXEN) | (1<<TXEN);
	// UCSRC- Setting UCSZ1:0 to '1' gives us an 8-bit frame size. There
	//   are provisions in this register for synchronous mode, parity,
	//   and stop-bit count, but we'll ignore them.
	UCSRC = (1<<UCSZ1) | (1<<UCSZ0);

	// set_sleep_mode() is a nice little macro from the sleep library which
	//   sets the stage nicely for sleep; after this, all you need to do is
	//   call sleep_mode() to put the processor to sleep. Power Down mode is
	//   the lowest possible sleep power state; all clocks are stopped and
	//   only an external interrupt can wake the processor.
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
	// Check to make sure the EEPROM has been configured for first use by
	//   looking at the "key" location. If not, configure it.
	if (EEPROMReadByte((uint8_t)KEY_ADDR) != KEY) EEPROMConfig();
	
	// EEPROMRetrieve() pulls the various operational parameters out of
	//   EEPROM and puts them in SRAM.
	EEPROMRetrieve();
	
	// Configure the ADXL362 with the info we just pulled from EEPROM.
	ADXLConfig();

	// We need to set up Timer1 as an overflow interrupt so the device can
	//   stay awake long enough for the user to input some parameters.
	//   Timer1 is a 16-bit counter; I'm going to set it up so that it ticks
	//   over every 10 seconds when the device is awake, and when it ticks,
	//   the device drops back into sleep.
	// TCCR1B- 101 in CS1 bits divides the clock by 1024; ~one count per ms.
	TCCR1B = (1<<CS12) | (0<<CS11) | (1<<CS10);
	// TCNT1- When this hits 65,536, an overflow interrupt occurs. By
	//   "priming" it, we reduce the time until an interrupt occurs.
	//   The if/else is to prevent the user accidentally
	//   setting it so low that the part goes back to sleep before it can be
	//   reprogrammed by the user through the command line.
	TCNT1 = t1Offset;
	// TIMSK- Set TOIE1 to enable Timer1 overflow interrupt
	TIMSK = (1<<TOIE1);
	
	// loadOn() is a simple function that turns on the load. We'll turn it on
	//   now and leave it on until sleep.
	loadOn();
		
	// sei() is a macro that basically executes the single instruction
	//   global interrupt enable function. Up until now, interrupt sources
	//   are ignored. We will turn this on and leave it on, unless we need
	//   to perform some sensitive tasks which require interrupts disabled
	//   for a time.
	sei();
	
	// Okay, now we can move forward with our main application code. This loop
	//   will just run over and over forever, waiting for signals from interrupts
	//   to tell it what to do.
	printMenu();
	while(1)
	{
		// The main functionality is to go to sleep when there's been no activity
		//   for some time; if Timer1 manages to overflow, it will set sleepyTime
		//   true.
		if (sleepyTime == TRUE)
		{
			serialWrite("z");			// Let the user know sleep mode is coming.
			ADXLConfig();
			GIMSK = (1<<INT0) |(1<<INT1);// Enable external interrupts to wake the
										//   processor up; INT0 is incoming serial
										//   data, INT1 is accelerometer interrupt
			loadOff();					// Turn off the load for sleepy time.
			sleep_mode();				// Go to sleep until awoken by an interrupt.
			EEPROMRetrieve();			// Retrieve EEPROM values, mostly to print
										//   them out to the user, if the wake-up
										//   was due to serial data arriving.
			printMenu();
			loadOn();					// Turn the load back on.
		}
		// Any data arriving over the serial port will trigger a serial receive
		//   interrupt. If that data is non-null, serialParse() will be called to
		//   deal with it.
		if (serialRxData != 0) serialParse();
	}
}

// Utility function which pulls the various operational paramters out of EEPROM,
//   puts them into SRAM, and prints them over the serial line.
void EEPROMRetrieve(void)
{
	uint16_t threshold = EEPROMReadWord((uint8_t)ATHRESH);		// Activity threshold. See
														//   ADXL362 datasheet for info.
	t1Offset =  EEPROMReadWord((uint8_t)WAKE_OFFS);     // (65535 - t1Offset)ms elapse
														//   between Timer1 interrupts.
	serialWriteInt(threshold);							// Print threshold in human format.
	serialWriteInt(65535 - t1Offset);					// Print the delay before sleep
														//   in human format.
}

// Configuration function for EEPROM- "erased" for the EEPROM is 65535, so we need to
//   change these to more manageable values the first time the board powers up, or the
//   sleep interrupt will happen WAY too fast and the motion threshold will be WAY too
//   high for practicality.
void EEPROMConfig(void)
{
	t1Offset  = 60535;		// Corresponds to ~5s delay before going to sleep
	// Now let's store these, along with the "key" that let's us know we've done this.
	EEPROMWriteWord((uint8_t)ATHRESH, (uint16_t) 150);
	EEPROMWriteWord((uint8_t)WAKE_OFFS, (uint16_t)t1Offset);
	EEPROMWriteWord((uint8_t)ITHRESH, (uint16_t)50);
	EEPROMWriteWord((uint8_t)ITIME, (uint16_t)15);
	EEPROMWriteByte((uint8_t)KEY_ADDR, (uint8_t)KEY);
}

