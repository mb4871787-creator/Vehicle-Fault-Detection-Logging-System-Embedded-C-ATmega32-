 /******************************************************************************
 *
 * Module: ADC
 *
 * File Name: adc.c
 *
 * Description: Source file for the ATmega16 ADC driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#include "avr/io.h" /* To use the ADC Registers */
#include "adc.h"
#include "common_macros.h" /* To use the macros like SET_BIT */

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

volatile float g_adcVref = 2.56; // default

void ADC_init(const ADC_ConfigType * Config_Ptr)
{
	if(Config_Ptr!=NULL_PTR){
		switch (Config_Ptr->ref_volt)
			{
			case AREF:
				ADMUX &= ~((1<<REFS1) | (1<<REFS0));    /* AREF, internal Vref turned off */
				g_adcVref = 5.0;
				break;
			case AVCC:
				ADMUX = (ADMUX & ~((1<<REFS1)|(1<<REFS0))) | (1<<REFS0);  /* AVCC */
				g_adcVref = 5.0;
				break;
			case INTERNAL_2_56V:
				ADMUX |= (1<<REFS1) | (1<<REFS0);       /* Internal 2.56V reference */
				g_adcVref = 2.56;
				break;
			}

			/* -------- Prescaler Selection (ADCSRA ADPS2:0) -------- */
			ADCSRA &= ~((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)); /* clear bits first */
			switch (Config_Ptr->prescaler)
			{
			case ADC_PRESCALER_2:
				ADCSRA |= (1<<ADPS0);
				break;
			case ADC_PRESCALER_4:
				ADCSRA |= (1<<ADPS1);
				break;
			case ADC_PRESCALER_8:
				ADCSRA |= (1<<ADPS1)|(1<<ADPS0);
				break;
			case ADC_PRESCALER_16:
				ADCSRA |= (1<<ADPS2);
				break;
			case ADC_PRESCALER_32:
				ADCSRA |= (1<<ADPS2)|(1<<ADPS0);
				break;
			case ADC_PRESCALER_64:
				ADCSRA |= (1<<ADPS2)|(1<<ADPS1);
				break;
			case ADC_PRESCALER_128:
				ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
				break;
			}

			/* Enable ADC (ADEN = 1) */
			ADCSRA |= (1<<ADEN);
	}

}

uint16 ADC_readChannel(uint8 channel_num)
{
	channel_num &= 0x07; /* Input channel number must be from 0 --> 7 */
	ADMUX &= 0xE0; /* Clear first 5 bits in the ADMUX (channel number MUX4:0 bits) before set the required channel */
	ADMUX = ADMUX | channel_num; /* Choose the correct channel by setting the channel number in MUX4:0 bits */
	SET_BIT(ADCSRA,ADSC); /* Start conversion write '1' to ADSC */
	while(BIT_IS_CLEAR(ADCSRA,ADIF)); /* Wait for conversion to complete, ADIF becomes '1' */
	SET_BIT(ADCSRA,ADIF); /* Clear ADIF by write '1' to it :) */
	return ADC; /* Read the digital value from the data register */
}
