/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

ui.cpp
This file contains implementations of the various user interface functionality.
Primarily, it contains a very large state machine for parsing data received
across the serial port into commands.
******************************************************************************/

#include<avr/io.h>
#include "ui.h"
#include "wake-on-shake.h"
#include "eeprom.h"
#include "serial.h"
#include "ADXL362.h"

extern uint16_t				t1Offset;		// see Wake-on-Shake.cpp
extern volatile uint8_t		serialRxData;	// see Wake-on-Shake.cpp

// Probably the most complex part of the code, serialParse() is a state machine
//   which provides a limited user interface for setting and getting the parameters
//   which dictate the operation of the Wake-on-shake. It gets called by the main
//   code whenever a serial interrupt receives a non-null value over the serial
//   port. It echoes back the received data (primarily for user convenience), then
//   handles the data depending on current state and received data.
void serialParse(void)
{
	// Static variables- these need to persist between calls to this function.
	
	// inputBufferValue is the number that the user is currently entering in-
	//  it could be a threshold value, a delay value, a value to be written
	//  to the ADXL362 or into EEPROM, or something else. We parse it in by
	//  multiplying the old value by ten and adding the recently received value
	//  to the previous value, so we need the current value to persist between
	//  calls.
	static uint16_t		inputBufferValue = 0;
	// mode lets the function know how it should respond to data. In some cases,
	//   we just want to keep incrementing inputBufferValue; in others, we want
	//   to take immediate action.
	static uint8_t		mode = ' ';
	// serialDataBuffer is used to store a value the user wants to either send
	//   to the ADXL362 or put into EEPROM. For ease of implementation, we only
	//   do one byte at a time- first put in the data, then tell the device
	//   where to send it.
	static uint8_t		serialDataBuffer = 0;
	// localData buffers serialRxData. Because of the length of time it can
	//   take for this function to complete (at 2400 baud, serial writes take
	//   quite some time, as do EEPROM writes), we want to know that nothing
	//   has changed between when we enter the function and code we encounter
	//   further down. The only way to ensure that is to buffer serialRxData
	//   locally, since serialRxData can be changed by the serial receive ISR
	//   at any time.
	uint8_t				localData = serialRxData;
	
	serialRxData = 0;					// Clear serialRxData
	//serialWriteChar(localData);		// Echo received data. Removed from
										//  released version b/c there's no
										//  real need to do this.
	
// The state machine is implemented as an if/else group.
	
// First case: carriage returns/newlines when we're in the "no data" state.
//   Basically, we can just ignore these. This is important, because we don't
//   know for sure what the serial program the user is entering data from will
//   send at the end of the line, so we need to respond to either if they
//   happen at the end of data entry, and ignore them any other time, since
//   some programs may send BOTH CR and LF. We also don't want to let this be
//   the default case, because it would be annoying to see "Invalid input"
//   every time we enter a value.
	if (((localData == '\n') | (localData == '\r')) & (mode == ' '));
	
// CR/LF when we're in data reading mode: respond to whichever one happens first
//   by parsing the received data.
	else if (((localData == '\n') | (localData == '\r')) & (mode != ' '))
	{
		// Now, make a decision about what to do with the data depending on the mode.
		switch(mode)
		{
			// 't' indicates that user wanted to change the threshold setting, so
			//   let's store the inputBufferValue in EEPROM and update the value in
			//   memory. We won't bother updating the ADXL362 just yet; that will
			//   happen right before we go to sleep.
			case 't':
			EEPROMWriteWord((uint8_t)ATHRESH, inputBufferValue);
			break;
			// 'd' indicates that user wanted to change the delay before sleep, so
			//   so we need to convert the user's value in milliseconds to an offset
			//   value that can be loaded into TCNT1. We'll also include a check so
			//   the user can't accidentally set the timeout period so short as to
			//   render the device difficult to program.
			case 'd':
			t1Offset = 65535 - inputBufferValue;
			if (t1Offset > 63535) t1Offset = 63535;
			EEPROMWriteWord((uint8_t)WAKE_OFFS, t1Offset);
			break;
			// 'b' indicates that the user wishes to buffer a value to be written
			//   to something, either the ADXL362 -or- an EEPROM location in the tiny.
			case 'b':
			serialDataBuffer = (uint8_t)inputBufferValue;
			break;
			// 'w' indicates that the user wants to write a value directly to the
			//   ADXL362 part. In this case, inputBufferValue is taken as an
			//   address to write to, and the data to be written is in the
			//   serialDataBuffer variable.
			case 'w':
			ADXLWriteByte((uint8_t)inputBufferValue, serialDataBuffer);
			break;
			// 'r' indicates a desire to read from an address in the ADXL part.
			//   inputBufferValue provides an address to read from.
			case 'r':
			serialWriteInt((uint16_t)ADXLReadByte((uint8_t)inputBufferValue));
			break;
			// 'e' directs the device to store the buffered value into the
			//   address provided by inputBufferValue.
			case 'e':
			EEPROMWriteByte((uint8_t)inputBufferValue, serialDataBuffer);
			break;
			// 'E' directs the device to return a value stored in the EEPROM
			//   over the serial port from the address specified.
			case 'E':
			serialWriteInt((uint16_t)EEPROMReadByte((uint8_t)inputBufferValue));
			break;
		}
		inputBufferValue = 0;		// Clear the input buffer for next data stream.
		mode = ' ';					// Reset the mode for next data stream.
		printMenu();				// Just an indicator of success.
	}
	
// If the mode is currently null, and the character entered is a valid mode,
//   let's activate that mode. In some cases, we want immediate action, and
//   then to return to null mode.
	else if ((mode == ' ')&(
			(localData == 't') |	// Change the threshold setting
			(localData == 'd') |	// Change the delay before sleep
			(localData == 'z') |	// Force sleep in ~35ms
			(localData == 'b') |	// Buffer a byte for EEPROM or ADXL write
			(localData == 'w') |	// Write buffered byte to ADXL362 register
			(localData == 'r') |	// Read ADXL362 register
			(localData == 'e') |	// Write buffered byte to EEPROM address
			(localData == 'E') |	// Read byte from EEPROM address
			(localData == 'p') |	// Read pin level (pins on header only)
			(localData == 'H') |	// Set pin high (pins on header only)
			(localData == 'L')		// Set pin low (pins on header only)
			))
	{
		mode = localData;
		// Most of the time, we need more information from the user. SOMETIMES,
		//   we may want to take immediate action. ATM, only a 'z' spurs that,
		//   indicating to the device that we want it to go to sleep post-haste
		//   and not buffer more data. Future immediate commands should be
		//   added here.
	    if (mode == 'z')
		{
			TCNT1 = 65500;	// Set TCNT1 to *almost* overflowing. Sleep will
							//   occur right after an overflow of TCNT1.
			mode = ' ';		// Clear mode for later.
		}
	}
// Mode handler. Depending on the mode, the current input character should
//   be handled differently. Code for handling what happens before a user
//   has finished inputting a number goes here.
	else if (	(mode == 't')|\
				(mode == 'd')|\
				(mode == 'w')|\
				(mode == 'r')|\
				(mode == 'e')|\
				(mode == 'E')|\
				(mode == 'p')|\
				(mode == 'H')|\
				(mode == 'L')|\
				(mode == 'b'))
	{
		// case 'p': indicates user wants to print out the state of a given
		//   pin. Pins available for this are PB0:3 and PD6; we'll respond
		//   based on a numerical input 0-3 or 6. Other values print an
		//   error message. Note that no cr/lf is needed to complete this
		//   operation- it just...ends, as soon as another character has
		//   been received.
		if (mode == 'p')
		{
			mode = ' ';   // clear mode. We'll do this regardless of outcome.
			switch(localData)
			{
				case '0':
				DDRB &= ~(1<<PB0);							// make pin input
				serialWriteChar('0'+((PINB & (1<<PB0))>>PB0));    // isolate bit and
														    //   read it out
				break;
				case '1':
				DDRB &= ~(1<<PB1);
				serialWriteChar('0'+((PINB & (1<<PB1))>>PB1));
				break;
				case '2':
				DDRB &= ~(1<<PB2);
				serialWriteChar('0'+((PINB & (1<<PB2))>>PB2));
				break;
				case '3':
				DDRB &= ~(1<<PB3);
				serialWriteChar('0'+((PINB & (1<<PB3))>>PB3));
				break;
				case '6':
				DDRD &= ~(1<<PD6);
				serialWriteChar('0'+((PIND & (1<<PD6))>>PD6));
				break;
				default:
				abortInput();
				break;
			}
		}
		// case 'H': indicates user wants to set the state of a given pin to
		//   high. Pins available for this are PB0:3 and PD6; we'll respond
		//   based on a numerical input 0-3 or 6. Other values print an
		//   error message.
		else if (mode == 'H')
		{
			mode = ' ';   // clear mode. We'll do this regardless of outcome.
			switch(localData)
			{
				case '0':
				DDRB |= (1<<PB0);	// make pin an output
				PORTB |= (1<<PB0);  // set pin high
				break;
				case '1':
				DDRB |= (1<<PB1);
				PORTB |= (1<<PB1);
				break;
				case '2':
				DDRB |= (1<<PB2);
				PORTB |= (1<<PB2);
				break;
				case '3':
				DDRB |= (1<<PB3);
				PORTB |= (1<<PB3);
				break;
				case '6':
				DDRD |= (1<<PD6);
				PORTD |= (1<<PD6);
				break;
				default:
				abortInput();
				break;
			}
		}
		// case 'L': indicates user wants to set the state of a given pin to
		//   low. Pins available for this are PB0:3 and PD6; we'll respond
		//   based on a numerical input 0-3 or 6. Other values print an
		//   error message.
		else if (mode == 'L')
		{
			mode = ' ';   // clear mode. We'll do this regardless of outcome.
			switch(localData)
			{
				case '0':
				DDRB |= (1<<PB0);	// make pin an output
				PORTB &= ~(1<<PB0); // set pin low
				break;
				case '1':
				DDRB |= (1<<PB1);
				PORTB &= ~(1<<PB1);
				break;
				case '2':
				DDRB |= (1<<PB2);
				PORTB &= ~(1<<PB2);
				break;
				case '3':
				DDRB |= (1<<PB3);
				PORTB &= ~(1<<PB3);
				break;
				case '6':
				DDRD |= (1<<PD6);
				PORTD &= ~(1<<PD6);
				break;
				default:
				abortInput();
				break;
			}
		}
		// Case numeric value AND some other state not involving pins-
		//   store the value into input buffer.
		else if (('0' <= localData) & (localData <= '9'))
		{
			inputBufferValue *= 10;
			inputBufferValue += (localData - '0');
		}
		// Case ANYTHING ELSE entered by user- clear mode, and whine a bit
		//   so they know the screwed up.
		else
		{
			mode = ' ';
			abortInput();
		}
	}
	
// Otherwise, just ignore what's happening and go back to null mode.
	else 
	{
		mode = ' ';
		abortInput();
	}		
}

// Because of the nature of the processor, strings constants use up a 
//   disproportionate amount of flash memory. Therefore, it makes sense to 
//   minimize string constant storage in memory by writing a single function
//   to handle frequently called string constant print calls, even if it
//   would be simpler (and tidier) to put it inline.
void printMenu(void)
{
	serialWrite(":-)");
}

void abortInput(void)
{
	serialWrite(":-(");
}