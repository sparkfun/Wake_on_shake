#ifndef _wake_on_shake_h_included
#define _wake_on_shake_h_included
	  
uint8_t spiXfer(unsigned char);
void EEPROMRetrieve(void);
void serialParse(void);
void loadOn(void);
void loadOff(void);
void EEPROMConfig(void);

#define THRESH		0
#define WAKE_OFFS	2
#define KEY_ADDR	15
#define KEY         0xAA

#define loadOff() PORTD &= !(1<<PD4)
#define loadOn()  PORTD |= (1<<PD4)

#endif
