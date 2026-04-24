 /******************************************************************************
 *
 * Module: Temperature Sensor
 *
 * File Name: lm35_sensor.h
 *
 * Description: source file for the LM35 Temperature Sensor driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#include "lm35_sensor.h"
#include "adc.h"

/*
 * Description :
 * Function responsible for calculate the temperature from the ADC digital value.
 */

extern float g_adcVref;

uint8 LM35_getTemperature(void)
{
	uint8 temp_value = 0;

	uint16 adc_value = 0;

	/* Read ADC channel where the temperature sensor is connected */
	adc_value = ADC_readChannel(LM35_CHANNEL_ID);

	/* Calculate the temperature from the ADC value*/
	//T=  adc_value(vRef/1023)*(1.5/150)
	temp_value = (uint8)(((uint32)adc_value * g_adcVref * 100) / (ADC_MAXIMUM_VALUE));

	return temp_value;
}

