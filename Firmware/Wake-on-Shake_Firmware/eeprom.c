/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

eeprom.cpp
Function implementations for handling the EEPROM. Rolled my own because it
didn't seem that the built-in functions were working; in retrospect that's
probably not the case, but done is done.
******************************************************************************/

#include <avr/io.h>
#include "eeprom.h"
#include <avr/interrupt.h>
#include "serial.h"
#include "wake-on-shake.h"

extern uint16_t				t1Offset;  // See Wake-on-Shake.cpp

// Write a 16-bit value to EEPROM. Data is written big-endian. Note that
//   blocking while waiting for prior writes to EEPROM to complete is
//   handled in the byte read/write calls, which are called from here,
//   so blocking is NOT needed in this function.
void EEPROMWriteWord(uint8_t addr, uint16_t data)
{
	uint16_t dataTemp = data>>8;				// Isolate the high byte.
	EEPROMWriteByte(addr, (uint8_t)dataTemp);   // Write high byte to EEPROM.
	EEPROMWriteByte(addr+1, (uint8_t)data);		// Write low byte to EEPROM.
}

// Read a 16-bit value from EEPROM. Data is written big-endian. Note that
//   blocking while waiting for prior writes to EEPROM to complete is
//   handled in the byte read/write calls, which are called from here,
//   so blocking is NOT needed in this function.
uint16_t EEPROMReadWord(uint8_t addr)
{
	uint16_t readResult;
	readResult = (uint16_t)EEPROMReadByte(addr);	// Retrieve the high byte.
	readResult = readResult<<8;						// Left shift the hight byte.
	readResult |= (uint16_t)EEPROMReadByte(addr+1); // OR-in the low byte.
	return readResult;
}

// 8-bit write to EEPROM. Since EEPROM writes can take rather a long time, we
//   want to disable interrupts to avoid any unforeseen register mashing.
void EEPROMWriteByte(uint8_t addr, uint8_t data)
{
	cli();							// Disable interrupts.
	while (EECR & (1<<EEPE));		// Wait for in-progress EEPROM writes to
									//  complete before using their resources.
	EECR = (0<<EEPM1) | (0<<EEPM0); // See datasheet for details on the hows
	EEAR = addr;					//  and whys of this write process.
	EEDR = data;
	EECR |= (1<<EEMPE);
	EECR |= (1<<EEPE);
	sei();							// Re-enable interrupts once the write is
									//  underway- they can't hurt the process.
}

// 8-bit read from EEPROM. The read needs to be atomic, so we want to disable
//   interrupts before starting it up to keep registers intact.
uint8_t EEPROMReadByte(uint8_t addr)
{
	cli();						// Disable interrupts.
	while (EECR & (1<<EEPE));	// Wait for any writes to finish, to avoid
								//   a resource conflict.
	EEAR = addr;				// See the datasheet for more details about
	EECR += (1<<EERE);			//  this process.
	sei();						// Re-enable the interrupts.
	return EEDR;				// Return the value at the address in question.
}
