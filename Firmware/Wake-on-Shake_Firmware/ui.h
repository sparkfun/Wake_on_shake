/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

ui.h

******************************************************************************/

#ifndef UI_H_
#define UI_H_

void serialParse(void);		// This function parses user input data from the
							//   serial port into usable information for the
							//   processor.
void printMenu(void);		// Prints the string "Ready!". That's the menu.
void abortInput(void);		// Prints the string "Bad input!".

#endif /* UI_H_ */