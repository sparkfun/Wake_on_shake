/*
 * Wake_on_Shake.cpp
 *
 * Created: 11/13/2012 2:30:23 PM
 *  Author: mike.hord
 */ 

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "eeprom.h"
#include "wake-on-shake.h"
#include "interrupts.h"

uint16_t			t1Offset;
uint16_t			x_threshold;
uint16_t			y_threshold;
uint16_t			z_threshold;
volatile uint16_t	timeAwake = 0;
volatile bool		sleepyTime = false;
volatile uint8_t	mode = ' ';
volatile uint16_t   inputBufferValue = 0;
volatile uint8_t    serialRxData = 0;

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
	//   Also, PD2 should be made high to enable pullup resistor for when no
	//   serial connection is present.
	PORTD = (1<<PD6) | (1<<PD5) | (1<<PD2);
	
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
	
	// We need to set up Timer1 as an overflow interrupt so the device can
	//   stay awake long enough for the user to input some parameters.
	//   Timer1 is a 16-bit counter; I'm going to set it up so that it ticks
	//   over every 10 seconds when the device is awake, and when it ticks,
	//   the device drops back into sleep.
	// TCCR1B- 101 in CS1 bits divides the clock by 1024; one count per ms.
	TCCR1B = (1<<CS12) | (0<<CS11) | (1<<CS10);
	// TIMSK- Set TOIE1 to enable Timer1 overflow interrupt
	TIMSK = (1<<TOIE1);
	
	// Set up the USART peripheral for writing in and out. This isn't used
	//   during normal operation- only during user configuration. We'll
	//   set the mode to 2400 baud, 8-N-1. Pretty standard, really.

	// For 2400 baud, at 1.000MHz (which is our clock speed, since we're
	//   using the internal oscillator clocked down), UBRR should be set to
	//   25, and the U2X bit of UCSRA should be set to '0'.
	UBRRH = 0;
	UBRRL = 25;
	UCSRA = (0<<U2X);
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
	
	serialWrite("poop");
	EEPROMRetrieve();
	// Now, we're going to retrieve the X, Y, and Z threshold values, along
	//   with the time awake value, from EEPROM.

	inputBufferValue = 0;
	mode = ' ';
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sei();
	
	// TCNT1- When this hits 65,536, an overflow interrupt occurs. By
	//   "priming" it to 55,000, we reduce the time until an interrupt to
	//   roughly 10 seconds.
	TCNT1 = 55000;
	t1Offset = 60000;
	while(1)
	{
		if (sleepyTime)
		{
			serialWrite("z");
			GIMSK = (1<<INT0);
			sleep_mode();
			EEPROMRetrieve();
			TCNT1 = 55000;
		}
		if (serialRxData != 0) serialParse();
	}
}

void ADXLCheck(void)
{
	unsigned char temp = 0x0B;
	PORTB &= 0b11101111;
	spiXfer(temp);
	temp = 0;
	spiXfer(temp);
	temp = spiXfer(temp);
	PORTB |= (1<<PB4);
	serialWriteInt((unsigned int)temp);
}

// spiXfer() takes a byte and sends it out via the USI function. It does
//   NOT handle the chip select; user must do that before calling spiXfer().
//   It returns the value that was shifted in.

unsigned char spiXfer(unsigned char data)
{
	USIDR = data;
	while ((USISR & (1<<USIOIF)) == 0)
	{
		USICR = (0<<USIWM1) | (1<<USIWM0) | (1<<USICS1) | (0<<USICS0) | (1<<USICLK) | (1<<USITC);
	}
	USISR = (1<<USIOIF);
	return USIDR;
}

void EEPROMRetrieve(void)
{
	x_threshold = EEPROMReadWord((uint8_t)X_THRESH);
	y_threshold = EEPROMReadWord((uint8_t)Y_THRESH);
	z_threshold = EEPROMReadWord((uint8_t)Z_THRESH);
	t1Offset =    EEPROMReadWord((uint8_t)WAKE_OFFS);
	serialWriteInt(x_threshold);
	serialWrite(" ");
	serialWriteInt(y_threshold);
	serialWrite(" ");
	serialWriteInt(z_threshold);
	serialWrite(" ");
	//if (t1Offset > 55000) t1Offset = 55000;
	serialWriteInt(t1Offset);
	serialWrite(" ");
}

void serialParse(void)
{
	serialWriteChar(serialRxData);
	if (((serialRxData == '\n') | (serialRxData == '\r')) & (mode == ' ')) serialWriteChar('k');
	else if (((serialRxData == '\n') | (serialRxData == '\r')) & (mode != ' '))
	{
		serialWriteInt(inputBufferValue);
		serialWriteChar(mode);
		switch(mode)
		{
			case 'x':
			EEPROMWriteWord((uint8_t)X_THRESH, inputBufferValue);
			x_threshold = inputBufferValue;
			break;
			case 'y':
			EEPROMWriteWord((uint8_t)Y_THRESH, inputBufferValue);
			y_threshold = inputBufferValue;
			break;
			case 'z':
			EEPROMWriteWord((uint8_t)Z_THRESH, inputBufferValue);
			z_threshold = inputBufferValue;
			break;
			case 'd':
			t1Offset = 65535 - inputBufferValue;
			EEPROMWriteWord((uint8_t)WAKE_OFFS, t1Offset);
			break;
		}
		inputBufferValue = 0;
		mode = ' ';
	}
	else if ((mode == ' ')&((serialRxData == 'x') | (serialRxData == 'y') | (serialRxData == 'z') | (serialRxData == 'd')))
	{
		mode = serialRxData;
	}
	else if ((mode == 'x') | (mode == 'y') | (mode == 'z') | (mode == 'd'))
	{
		if ((47<serialRxData) & (serialRxData<58))
		{
			inputBufferValue *= 10;
			inputBufferValue += (serialRxData - 48);
		}
	}
	else mode = ' ';
	serialRxData = 0;
}
