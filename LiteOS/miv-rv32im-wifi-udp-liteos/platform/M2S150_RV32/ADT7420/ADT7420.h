/***************************************************************************//**
 * (c) Copyright 2017 Microsemi Corporation.  All rights reserved.
 * 
 *  Analog device ADT7420 Temperature sensor driver API.
 *
 * SVN $Revision:$
 * SVN $Date:$
 */
#ifndef __ADT7420_H__
#define __ADT7420_H__

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Displays Revision ID and Manufacturer ID. */
void ADT7420_PrintID(void);

/* Returns resolution of ADT7420 internal ADC. */
uint8_t ADT7420_GetResolution(void);

/* Reads the temperature data and converts it to Celsius degrees. */
float ADT7420_GetTemperature(void);

#endif	/* __ADT7420_H__ */
