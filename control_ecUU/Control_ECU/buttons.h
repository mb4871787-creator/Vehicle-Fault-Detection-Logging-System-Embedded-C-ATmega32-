/*
 * buttons.h
 *
 *  Created on: Oct 24, 2025
 *      Author: dell
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "gpio.h"

#define	BUTTONS_PORT			PORTC_ID
#define	MOTOR1_OPEN_PIN			PIN2_ID
#define	MOTOR1_CLOSE_PIN		PIN3_ID
#define	MOTOR2_OPEN_PIN			PIN4_ID
#define	MOTOR2_CLOSE_PIN		PIN5_ID

uint8 button_pressed(void);
void buttons_INIT(void);

#endif /* BUTTONS_H_ */
