/*
 * Ultrasonic.h
 *
 *  Created on: Oct 3, 2025
 *      Author: dell
 */

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_
#include"std_types.h"

#define ECHO_PORT			PORTD_ID
#define ECHO_PIN			PIN6_ID

#define TRIGGER_PORT		PORTD_ID
#define TRIGGER_PIN			PIN7_ID

void Ultrasonic_init(void);

void Ultrasonic_Trigger(void) ;

uint16 Ultrasonic_readDistance(void) ;

void Ultrasonic_edgeProcessing(void);


#endif /* ULTRASONIC_H_ */
