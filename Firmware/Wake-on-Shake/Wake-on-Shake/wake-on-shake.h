#ifndef _wake_on_shake_h_included
#define _wake_on_shake_h_included
	  
uint8_t spiXfer(unsigned char);
void ADXLCheck(void);
void EEPROMRetrieve(void);

#define X_THRESH		0
#define Y_THRESH		2
#define Z_THRESH		4
#define WAKE_OFFS		6
	
#endif
