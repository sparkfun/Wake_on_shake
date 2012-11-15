#ifndef _eeprom_h_included
#define _eeprom_h_included

uint16_t EEPROMReadWord(uint8_t);
void     EEPROMWriteWord(uint8_t, uint16_t);
uint8_t  EEPROMReadByte(uint8_t);
void     EEPROMWriteByte(uint8_t, uint8_t);

#endif
