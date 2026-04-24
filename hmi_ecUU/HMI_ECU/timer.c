#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile Timer_ModeType g_timerMode[3] = {TMR_MODE_NORMAL, TMR_MODE_NORMAL, TMR_MODE_NORMAL};

/* Callback pointers: one for overflow, one for compare for each timer */
static volatile void (*g_timerOvfCallback[3])(void) = {0};
static volatile void (*g_timerCmpCallback[3])(void) = {0};

volatile uint8 g_sec=0;

void Timer_init(const Timer_ConfigType * Config_Ptr)
{
	if(Config_Ptr!=NULL_PTR){
		uint8 clk=Config_Ptr->timer_clock;
		switch(Config_Ptr->timer_ID){
		case TIMER0_ID:

			if (Config_Ptr->timer_mode == TMR_MODE_NORMAL) {
				/* WGM01:0 = 0 0 -> Normal */
				TCCR0 &= ~((1<<WGM01)|(1<<WGM00));
				/* set initial */
				TCNT0 = (uint8)(Config_Ptr->timer_InitialValue & 0xFF);
				/* enable overflow interrupt */
				TIMSK |= (1<<TOIE0);

				g_timerMode[0]=TMR_MODE_NORMAL;
			} else { /* CTC */
				/* WGM01:0 = 1 0 -> CTC (compare) */
				TCCR0 &= ~(1<<WGM00);
				TCCR0 |= (1<<WGM01);
				/* set initial */
				TCNT0 = (uint8)(Config_Ptr->timer_InitialValue & 0xFF);
				/* set OCR0 */
				OCR0 = (uint8)(Config_Ptr->timer_compare_MatchValue & 0xFF);
				/* enable compare interrupt */
				TIMSK |= (1<<OCIE0);
				g_timerMode[0]=TMR_MODE_COMPARE;
			}
			TCCR0 &= ~((1<<CS02)|(1<<CS01)|(1<<CS00));
			if (clk == TMR_PRESCALER_1)   TCCR0 |= (1<<CS00);
			else if (clk == TMR_PRESCALER_8)  TCCR0 |= (1<<CS01);
			else if (clk == TMR_PRESCALER_64) TCCR0 |= (1<<CS01)|(1<<CS00);
			else if (clk == TMR_PRESCALER_256) TCCR0 |= (1<<CS02);
			else if (clk == TMR_PRESCALER_1024) TCCR0 |= (1<<CS02)|(1<<CS00);
			/* NO_CLOCK (0) -> stops timer (already cleared) */
			break;

		case TIMER1_ID:

			if (Config_Ptr->timer_mode == TMR_MODE_NORMAL) {
				/* WGM13:0 = 0 -> Normal mode */
				TCCR1A &= ~((1<<WGM11)|(1<<WGM10));
				TCCR1B &= ~((1<<WGM13)|(1<<WGM12));
				TCNT1 = (uint16)(Config_Ptr->timer_InitialValue & 0xFFFF);
				TIMSK |= (1<<TOIE1);
				g_timerMode[1]=TMR_MODE_NORMAL;
			} else { /* CTC using OCR1A as top (Clear Timer on Compare) */
				/* WGM13:0 = 0b0100 => WGM12 = 1, others 0 -> CTC with OCR1A as TOP */
				TCCR1A &= ~((1<<WGM11)|(1<<WGM10));
				TCCR1B &= ~((1<<WGM13));
				TCCR1B |= (1<<WGM12);
				TCNT1 = (uint16)(Config_Ptr->timer_InitialValue & 0xFFFF);
				OCR1A = (uint16)(Config_Ptr->timer_compare_MatchValue & 0xFFFF);
				/* enable compare A interrupt */
				TIMSK |= (1<<OCIE1A);
				g_timerMode[1]=TMR_MODE_COMPARE;
			}
            TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
            if (clk == TMR_PRESCALER_1)   TCCR1B |= (1<<CS10);
            else if (clk == TMR_PRESCALER_8)  TCCR1B |= (1<<CS11);
            else if (clk == TMR_PRESCALER_64) TCCR1B |= (1<<CS11)|(1<<CS10);
            else if (clk == TMR_PRESCALER_256) TCCR1B |= (1<<CS12);
            else if (clk == TMR_PRESCALER_1024) TCCR1B |= (1<<CS12)|(1<<CS10);
			break;

		case TIMER2_ID:

			if (Config_Ptr->timer_mode == TMR_MODE_NORMAL) {
				/* WGM21:0 = 0 0 -> Normal */
				TCCR2 &= ~((1<<WGM21)|(1<<WGM20));
				TCNT2 = (uint8)(Config_Ptr->timer_InitialValue & 0xFF);
				TIMSK |= (1<<TOIE2);
				g_timerMode[2]=TMR_MODE_NORMAL;
			} else {
				/* CTC: WGM21 = 1, WGM20 = 0 */
				TCCR2 &= ~(1<<WGM20);
				TCCR2 |= (1<<WGM21);
				TCNT2 = (uint8)(Config_Ptr->timer_InitialValue & 0xFF);
				OCR2 = (uint8)(Config_Ptr->timer_compare_MatchValue & 0xFF);
				TIMSK |= (1<<OCIE2);
				g_timerMode[2]=TMR_MODE_COMPARE;
			}
            TCCR2 &= ~((1<<CS22)|(1<<CS21)|(1<<CS20));
            if (clk == TMR_PRESCALER_1)   TCCR2 |= (1<<CS20);
            else if (clk == TMR_PRESCALER_8)  TCCR2 |= (1<<CS21);
            else if (clk == TMR_PRESCALER_64) TCCR2 |= (1<<CS21)|(1<<CS20);
            else if (clk == TMR_PRESCALER_256) TCCR2 |= (1<<CS22);
            else if (clk == TMR_PRESCALER_1024) TCCR2 |= (1<<CS22)|(1<<CS20);

			break;

		}
	}
}

void Timer_deInit(Timer_ID_Type timer_type)
{
	switch (timer_type) {
	case TIMER0_ID:
		TCCR0 = 0;
		TIMSK &= ~((1<<TOIE0)|(1<<OCIE0));
		TCNT0 = 0;
		break;
	case TIMER1_ID:
		TCCR1A = 0;
		TCCR1B = 0;
		TIMSK &= ~((1<<TOIE1)|(1<<OCIE1A)|(1<<OCIE1B));
		TCNT1 = 0;
		break;
	case TIMER2_ID:
		TCCR2 = 0;
		TIMSK &= ~((1<<TOIE2)|(1<<OCIE2));
		TCNT2 = 0;
		break;
	}
}

/* Set callback. This sets compare callback if the timer was init in compare mode,
   otherwise it sets overflow callback. */
void Timer_setCallBack(void(*a_ptr)(void), Timer_ID_Type a_timer_ID )
{
	if (a_ptr != NULL_PTR){
		switch(a_timer_ID){
		case 0:
			if (g_timerMode[0]==TMR_MODE_NORMAL){
				g_timerOvfCallback[0]=a_ptr;
			}
			else{
				g_timerCmpCallback[0]=a_ptr;

			}

			break;
		case 1:
			if (g_timerMode[1]==TMR_MODE_NORMAL){
				g_timerOvfCallback[1]=a_ptr;
			}
			else{
				g_timerCmpCallback[1]=a_ptr;

			}

			break;
		case 2:
			if (g_timerMode[2]==TMR_MODE_NORMAL){
				g_timerOvfCallback[2]=a_ptr;
			}
			else{
				g_timerCmpCallback[2]=a_ptr;

			}

			break;
		}
	}


}

/* -------------------- ISRs -------------------- */
/* Timer0 Overflow */
ISR(TIMER0_OVF_vect)
{
	if (g_timerOvfCallback[TIMER0_ID]!=NULL_PTR) g_timerOvfCallback[TIMER0_ID]();
}

/* Timer0 Compare Match */
ISR(TIMER0_COMP_vect)
{
	if (g_timerCmpCallback[TIMER0_ID]!=NULL_PTR) g_timerCmpCallback[TIMER0_ID]();

}

/* Timer1 Overflow */
ISR(TIMER1_OVF_vect)
{
	if (g_timerOvfCallback[TIMER1_ID]!=NULL_PTR) g_timerOvfCallback[TIMER1_ID]();

}

/* Timer1 Compare A Match */
ISR(TIMER1_COMPA_vect)
{
	if (g_timerCmpCallback[TIMER1_ID]!=NULL_PTR) g_timerCmpCallback[TIMER1_ID]();

	g_sec++;

}

/* Timer2 Overflow */
ISR(TIMER2_OVF_vect)
{
	if (g_timerOvfCallback[TIMER2_ID]!=NULL_PTR) g_timerOvfCallback[TIMER2_ID]();

}

/* Timer2 Compare Match */
ISR(TIMER2_COMP_vect)
{
	if (g_timerCmpCallback[TIMER2_ID]!=NULL_PTR) g_timerCmpCallback[TIMER2_ID]();

}
