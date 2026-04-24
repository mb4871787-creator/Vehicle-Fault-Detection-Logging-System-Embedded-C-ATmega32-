/*
 * Ultrasonic.c
 *
 *  Created on: Oct 3, 2025
 *      Author: dell
 */
#include"ultrasonic.h"
#include"icu.h"
#include"gpio.h"
#include"util/delay.h"



static volatile uint8 g_edgeCount = 0;
static volatile uint16 g_timeHigh = 0;


void Ultrasonic_init(void){

	GPIO_setupPinDirection(TRIGGER_PORT, TRIGGER_PIN,PIN_OUTPUT);

	ICU_ConfigType ultrasonic;
	ultrasonic.clock=F_CPU_8;
	ultrasonic.edge=RAISING;


	ICU_init(&ultrasonic);
	_delay_ms(600);
	ICU_setCallBack(Ultrasonic_edgeProcessing);

}

void Ultrasonic_Trigger(void){

	GPIO_writePin(TRIGGER_PORT, TRIGGER_PIN, LOGIC_HIGH);
	_delay_us(10);
	GPIO_writePin(TRIGGER_PORT, TRIGGER_PIN, LOGIC_LOW);


}

uint16 Ultrasonic_readDistance(void){
	Ultrasonic_Trigger();

	return ((g_timeHigh/58.8)+1);

}


void Ultrasonic_edgeProcessing(void){
	g_edgeCount++;
		if(g_edgeCount == 1)
		{
			/*
			 * Clear the timer counter register to start measurements from the
			 * first detected rising edge
			 */
			ICU_clearTimerValue();
			/* Detect falling edge */
			ICU_setEdgeDetectionType(FALLING);
		}
		else if(g_edgeCount == 2)
		{
			/* Store the High time value */
			g_timeHigh = ICU_getInputCaptureValue();
			/* Detect rising edge */
			ICU_clearTimerValue();
			ICU_setEdgeDetectionType(RAISING);
			g_edgeCount=0;
		}
}
