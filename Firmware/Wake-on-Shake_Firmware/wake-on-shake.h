/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons 
Share Alike v3.0 license:
	http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask 
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

wake-on-shake.h
This is the main definitions file for the core code in the project. The few
functions in here are all basically convenience aggregations of calls to
functions in other places.
******************************************************************************/

#ifndef _wake_on_shake_h_included
#define _wake_on_shake_h_included
	  
void EEPROMRetrieve(void);	// Pulls config values out of EEPROM and displays.
void EEPROMConfig(void);	// Set the EEPROM storage locations up if they
							//   aren't already configured.

#define ATHRESH		0		// EEPROM address for the activity threshold.
#define WAKE_OFFS	2		// EEPROM address for the wake offset.
#define ITHRESH		4		// EEPROM address for the inactivity threshold.
#define ITIME		6		// EEPROM address for the activity time.
#define KEY_ADDR	127		// EEPROM address for the EEPROM configuration key.
#define KEY         123		// EEPROM configuration key value.

// Macros for turning the load on and off.
#define loadOff() PORTD &= !(1<<PD4)
#define loadOn()  PORTD |= (1<<PD4)

#define TRUE 1
#define FALSE 0

#endif
