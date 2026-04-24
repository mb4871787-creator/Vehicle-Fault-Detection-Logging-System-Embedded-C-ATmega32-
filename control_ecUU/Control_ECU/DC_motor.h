/*
 * DC_motor.h
 *
 *  Created on: Sep 27, 2025
 *      Author: dell
 */

#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_
#include"std_types.h"
#include "gpio.h"

#define MOTOR1			1
#define MOTOR2			2

#define MOTOR_PORT1		PORTB_ID
#define IN1				PIN1_ID
#define IN2				PIN2_ID
#define EN				PIN3_ID

#define MOTOR_PORT2		PORTB_ID
#define IN3				PIN4_ID
#define IN4				PIN5_ID


#define STOP			0
#define ACW				1
#define CW				2

//#ifdef anti_ckw
//
//#define IN1_DIRECTION  LOGIC_HIGH
//#define IN2_DIRECTION  LOGIC_LOW
//
//#endif
//
//#ifdef ckw
//
//#define IN1_DIRECTION  LOGIC_LOW
//#define IN2_DIRECTION  LOGIC_HIGH
//
//#endif

void DcMotor_Init(void);

void DcMotor_Rotate(uint8 state, uint8 speed, uint8 motor_id);

void DcMotor_DeInit(void);

#endif /* DC_MOTOR_H_ */
