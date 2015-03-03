/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

eeprom.h
Function definitions for the eeprom.
******************************************************************************/

#ifndef _eeprom_h_included
#define _eeprom_h_included

uint16_t EEPROMReadWord(uint8_t);				// 16-bit read from EEPROM.
void     EEPROMWriteWord(uint8_t, uint16_t);	// 16-bit write to EEPROM.
uint8_t  EEPROMReadByte(uint8_t);				// 8-bit read from EEPROM.
void     EEPROMWriteByte(uint8_t, uint8_t);		// 8-bit write to EEPROM.

#endif
