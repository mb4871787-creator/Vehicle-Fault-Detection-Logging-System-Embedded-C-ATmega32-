/*
 * timer.h
 *
 *  Created on: Oct 18, 2025
 *      Author: dell
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "std_types.h"

/* If F_CPU not defined by build system, define a default here */
#ifndef F_CPU
#define F_CPU 8000000UL
#endif


/* Timer clock / prescaler options (only common ones included) */
typedef enum {
    TMR_NO_CLOCK = 0,
    TMR_PRESCALER_1,
    TMR_PRESCALER_8,
    TMR_PRESCALER_64,
    TMR_PRESCALER_256,
    TMR_PRESCALER_1024,
    /* External clocks not used here but could be added */
} Timer_ClockType;

/* Timer mode */
typedef enum {
    TMR_MODE_NORMAL = 0,
    TMR_MODE_COMPARE   /* CTC for 8-bit/16-bit timers */
} Timer_ModeType;

/*Timer ID*/
typedef enum {
    TIMER0_ID,
	TIMER1_ID,
	TIMER2_ID
} Timer_ID_Type;

/* Config structure as required */
typedef struct
{
  uint16 timer_InitialValue;
  uint16 timer_compare_MatchValue;     /* used in compare mode only */
  Timer_ID_Type  timer_ID;
  Timer_ClockType timer_clock;
  Timer_ModeType  timer_mode;
} Timer_ConfigType;

/* API */
void Timer_init(const Timer_ConfigType * Config_Ptr);
void Timer_deInit(Timer_ID_Type timer_type);
void Timer_setCallBack(void(*a_ptr)(void), Timer_ID_Type a_timer_ID );

void timer1_callback(void);
extern volatile uint8 g_sec;






#endif /* TIMER_H_ */
