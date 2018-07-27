/***************************************************************************//**
 * (c) Copyright 2017 Microsemi Corporation.  All rights reserved.
 * 
 *  Analog device ADT7420 Temperature sensor driver implementation.
 *
 * SVN $Revision:$
 * SVN $Date:$
 */
 
#include <stdio.h>
#include <math.h>
#include "core_i2c.h"

/*-----------------------------------------------------------------------------
 * I2C slave serial address.
 */
#define SLAVE_SER_ADDR     0x4B

#define RESOLUTION 			7

/*------------------------------------------------------------------------------
 * Instance data for CoreI2C devices
 */
i2c_instance_t g_core_i2c;

uint8_t resolutionSetting = 0;    // Current resolution setting

/******************************************************************************
* @brief Displays Revision ID and Manufacturer ID.
*
* @param None.
*
* @return None.
******************************************************************************/
void ADT7420_PrintID(void)
{
	uint8_t rxBuffer[1] = {0x00};
	volatile i2c_status_t status;
	uint8_t idregisteraddr = 0x0B;
	uint8_t read_id[1] = {0};

    I2C_write_read( &g_core_i2c, SLAVE_SER_ADDR, &idregisteraddr, 1,
    		        &read_id, 1, I2C_RELEASE_BUS );

    status = I2C_wait_complete( &g_core_i2c, I2C_NO_TIMEOUT );

}

/******************************************************************************
* @brief Returns resolution of ADT7420 internal ADC.
*
* @param display - 0 -> resolution is displayed on UART
*				 - 1 -> resolution is not displayed on UART.
*
* @return (rxBuffer[0] & (1 << RESOLUTION)) - bit 7 of CONFIGURATION REGISTER
* 				 - 0 -> resolution is 13 bits
* 				 - 1 -> resolution is 16 bits.
******************************************************************************/
uint8_t ADT7420_GetResolution(void)
{
	unsigned char rxBuffer[2] = {0x00, 0x00};
	volatile i2c_status_t status;
	uint8_t configregister = 0x03;
	uint8_t resolution = 0;

    I2C_write_read( &g_core_i2c, SLAVE_SER_ADDR, &configregister, 1,
    		        &resolution, 1, I2C_RELEASE_BUS );

    status = I2C_wait_complete( &g_core_i2c, I2C_NO_TIMEOUT );

	return (resolution & (1 << RESOLUTION));
}

/***************************************************************************//**
 * @brief Reads the temperature data and converts it to Celsius degrees.
 *
 * @return temperature - Temperature in degrees Celsius.
*******************************************************************************/
float ADT7420_GetTemperature(void)
{
    uint8_t  msbTemp = 0;
    uint8_t  lsbTemp = 0;
    uint16_t temp    = 0;
    float          tempC   = 0;
    volatile i2c_status_t status;
    uint8_t tempMSBaddr = 0x00;
    uint8_t tempLSBaddr = 0x01;

    I2C_write_read( &g_core_i2c, SLAVE_SER_ADDR, &tempMSBaddr, 1,
    		        &msbTemp, 1, I2C_RELEASE_BUS );
    status = I2C_wait_complete( &g_core_i2c, I2C_NO_TIMEOUT );

    I2C_write_read( &g_core_i2c, SLAVE_SER_ADDR, &tempLSBaddr, 1,
        		        &lsbTemp, 1, I2C_RELEASE_BUS );
    status = I2C_wait_complete( &g_core_i2c, I2C_NO_TIMEOUT );

    temp    = ((uint16_t)msbTemp << 8) + lsbTemp;
    if(resolutionSetting)
    {
        if(temp & 0x8000)
        {
            /*! Negative temperature */
            tempC = (float)((signed long)temp - 65536) / 128;
        }
        else
        {
            /*! Positive temperature */
            tempC = (float)temp / 128;
        }
    }
    else
    {
        temp >>= 3;
        if(temp & 0x1000)
        {
            /*! Negative temperature */
            tempC = (float)((signed long)temp - 8192) / 16;
        }
        else
        {
            /*! Positive temperature */
            tempC = (float)temp / 16;
        }
    }

    return tempC;
}
