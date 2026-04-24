/*
 * DC_motor.c
 *
 *  Created on: Sep 27, 2025
 *      Author: dell
 */
#include"DC_motor.h"
#include <avr/io.h>
#include "pwm_t0.h"

void DcMotor_Init(void){
	GPIO_setupPinDirection( MOTOR_PORT1, IN1, PIN_OUTPUT);
	GPIO_setupPinDirection( MOTOR_PORT1, IN2, PIN_OUTPUT);

	/////
	GPIO_writePin( MOTOR_PORT1, IN1, LOGIC_LOW);
	GPIO_writePin( MOTOR_PORT1, IN2, LOGIC_LOW);




	GPIO_setupPinDirection( MOTOR_PORT2, IN3, PIN_OUTPUT);
	GPIO_setupPinDirection( MOTOR_PORT2, IN4, PIN_OUTPUT);

	/////
	GPIO_writePin( MOTOR_PORT2, IN3, LOGIC_LOW);
	GPIO_writePin( MOTOR_PORT2, IN4, LOGIC_LOW);




}

void DcMotor_Rotate(uint8 state, uint8 speed, uint8 motor_id){
	if(motor_id==MOTOR1){
		switch(state){
		case 0://stop
			GPIO_writePin( MOTOR_PORT1, IN1, LOGIC_LOW);
			GPIO_writePin( MOTOR_PORT1, IN2, LOGIC_LOW);

			break;
		case 1://anti-clockwise
			GPIO_writePin( MOTOR_PORT1, IN1, LOGIC_HIGH);
			GPIO_writePin( MOTOR_PORT1, IN2, LOGIC_LOW);
			break;
		case 2://clockwise
			GPIO_writePin( MOTOR_PORT1, IN1, LOGIC_LOW);
			GPIO_writePin( MOTOR_PORT1, IN2, LOGIC_HIGH);

			break;
		}
	}
	else{
		switch(state){
		case 0://stop
			GPIO_writePin( MOTOR_PORT2, IN3, LOGIC_LOW);
			GPIO_writePin( MOTOR_PORT2, IN4, LOGIC_LOW);

			break;
		case 1://anti-clockwise
			GPIO_writePin( MOTOR_PORT2, IN3, LOGIC_HIGH);
			GPIO_writePin( MOTOR_PORT2, IN4, LOGIC_LOW);

			break;
		case 2://clockwise
			GPIO_writePin( MOTOR_PORT2, IN3, LOGIC_LOW);
			GPIO_writePin( MOTOR_PORT2, IN4, LOGIC_HIGH);

			break;
		}
	}

			PWM_Timer0_Start(speed);
}


void DcMotor_DeInit(void)
{
    /* Stop both motors */
    GPIO_writePin(MOTOR_PORT1, IN1, LOGIC_LOW);
    GPIO_writePin(MOTOR_PORT1, IN2, LOGIC_LOW);
    GPIO_writePin(MOTOR_PORT1, EN, LOGIC_LOW);

    GPIO_writePin(MOTOR_PORT2, IN3, LOGIC_LOW);
    GPIO_writePin(MOTOR_PORT2, IN4, LOGIC_LOW);
    GPIO_writePin(MOTOR_PORT2, EN, LOGIC_LOW);

}

