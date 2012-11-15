#ifndef _interrupts_h_included
#define _interrupts_h_included

extern uint16_t				t1Offset;
extern uint16_t				x_threshold;
extern uint16_t				y_threshold;
extern uint16_t				z_threshold;
extern volatile bool		sleepyTime;
extern volatile uint8_t		mode;
extern volatile uint16_t	inputBufferValue;
extern volatile uint8_t     serialRxData;

#endif