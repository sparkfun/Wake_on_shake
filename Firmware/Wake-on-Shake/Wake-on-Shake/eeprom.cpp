#include <avr/io.h>
#include "eeprom.h"
#include <avr/interrupt.h>
#include "serial.h"
#include "wake-on-shake.h"

extern uint16_t				t1Offset;

void EEPROMWriteWord(uint8_t addr, uint16_t data)
{
	EEPROMWriteByte(addr, (uint8_t)data>>8);
	EEPROMWriteByte(addr+1, (uint8_t)data);
}

uint16_t EEPROMReadWord(uint8_t addr)
{
	uint16_t readResult;
	readResult = ((uint16_t)EEPROMReadByte(addr))<<8;
	readResult |= (uint16_t)EEPROMReadByte(addr+1);
	return readResult;
}

void EEPROMWriteByte(uint8_t addr, uint8_t data)
{
	TCNT1 = t1Offset;
	while (EECR & (1<<EEPE));
	EECR = (0<<EEPM1) | (0<<EEPM0);
	EEAR = addr;
	EEDR = data;
	EECR |= (1<<EEMPE);
	EECR |= (1<<EEPE);
}

uint8_t EEPROMReadByte(uint8_t addr)
{
	TCNT1 = t1Offset;
	while (EECR & (1<<EEPE));
	EEAR = addr;
	EECR += (1<<EERE);
	return EEDR;
}