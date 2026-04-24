/*
 * buttons.c
 *
 *  Created on: Oct 24, 2025
 *      Author: dell
 */
#include"buttons.h"
#include"gpio.h"
#include"std_types.h"
#include"common_macros.h"

void buttons_INIT(void){
	GPIO_setupPinDirection(BUTTONS_PORT,MOTOR1_OPEN_PIN,PIN_INPUT);
	GPIO_setupPinDirection(BUTTONS_PORT,MOTOR1_CLOSE_PIN,PIN_INPUT);
	GPIO_setupPinDirection(BUTTONS_PORT,MOTOR2_OPEN_PIN,PIN_INPUT);
	GPIO_setupPinDirection(BUTTONS_PORT,MOTOR2_CLOSE_PIN,PIN_INPUT);

	GPIO_writePin(BUTTONS_PORT,MOTOR1_OPEN_PIN,LOGIC_HIGH);
	GPIO_writePin(BUTTONS_PORT,MOTOR1_CLOSE_PIN,LOGIC_HIGH);
	GPIO_writePin(BUTTONS_PORT,MOTOR2_OPEN_PIN,LOGIC_HIGH);
	GPIO_writePin(BUTTONS_PORT,MOTOR2_CLOSE_PIN,LOGIC_HIGH);
}

uint8 button_pressed(void){
    if(GPIO_readPin(BUTTONS_PORT, MOTOR1_OPEN_PIN) == LOGIC_LOW){
        return 1;
    }
    else if(GPIO_readPin(BUTTONS_PORT, MOTOR1_CLOSE_PIN) == LOGIC_LOW){
        return 2;
    }
    else if(GPIO_readPin(BUTTONS_PORT, MOTOR2_OPEN_PIN) == LOGIC_LOW){
        return 3;
    }
    else if(GPIO_readPin(BUTTONS_PORT, MOTOR2_CLOSE_PIN) == LOGIC_LOW){
        return 4;
    }
    else{
        return 0;
    }
}
