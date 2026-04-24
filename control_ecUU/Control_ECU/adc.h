 /******************************************************************************
 *
 * Module: ADC
 *
 * File Name: adc.h
 *
 * Description: header file for the ATmega16 ADC driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#ifndef ADC_H_
#define ADC_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define ADC_MAXIMUM_VALUE    1023
//ADC_REF_VOLT_VALUE   2.56

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
typedef enum {
	AREF,             /* External AREF pin */
	AVCC,             /* AVCC with external capacitor at AREF pin */
	INTERNAL_2_56V    /* Internal 2.56V reference with capacitor at AREF pin */
} ADC_ReferenceVolatge;

typedef enum {
	ADC_PRESCALER_2 = 1,
	ADC_PRESCALER_4,
	ADC_PRESCALER_8,
	ADC_PRESCALER_16,
	ADC_PRESCALER_32,
	ADC_PRESCALER_64,
	ADC_PRESCALER_128
} ADC_Prescaler;


typedef struct{
      ADC_ReferenceVolatge ref_volt;
      ADC_Prescaler prescaler;
}ADC_ConfigType;
/*
 * Description :
 * Function responsible for initialize the ADC driver.
 */
void ADC_init(const ADC_ConfigType * Config_Ptr);

/*
 * Description :
 * Function responsible for read analog data from a certain ADC channel
 * and convert it to digital using the ADC driver.
 */
uint16 ADC_readChannel(uint8 channel_num);

#endif /* ADC_H_ */
