#include "los_sys.h"
#include "los_tick.h"
#include "los_task.ph"
#include "los_config.h"

#include "los_bsp_led.h"
#include "los_bsp_key.h"
#include "los_bsp_uart.h"
#include "los_inspect_entry.h"
#include "los_demo_entry.h"

#include "riscv_hal.h"
#include "hw_platform.h"
#include "core_i2c.h"
#include "adt7420.h"

#include <string.h>

/*------------------------------------------------------------------------------
 * I2C master serial address.
 */
#define MASTER_SER_ADDR     0x21

/*-----------------------------------------------------------------------------
 * I2C operation time-out value in mS. Define as I2C_NO_TIMEOUT to disable the
 * time-out functionality.
 */
#define DEMO_I2C_TIMEOUT 3000u

/*------------------------------------------------------------------------------
 * Instance data for CoreI2C devices
 */
extern i2c_instance_t g_core_i2c;


extern void LOS_EvbSetup(void);

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}


// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
   int i = 0;
   while (x)
   {
       str[i++] = (x%10) + '0';
       x = x/10;
   }

   // If number of digits required is more, then
   // add 0s at the beginning
   while (i < d)
       str[i++] = '0';

   reverse(str, i);
   str[i] = '\0';
   return i;
}

// Converts a floating point number to string.
void ftoa(float n, uint8_t *res, uint8_t afterpoint)
{
    // Extract integer part
	uint8_t ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    uint8_t i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * 10000;

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}
uint8_t External_27_IRQHandler(void)
{
    I2C_isr(&g_core_i2c);
    return (EXT_IRQ_KEEP_ENABLED);
}

uint8_t rcv_data[40] = {0};
static UINT32 g_uwboadTaskID;
LITE_OS_SEC_TEXT VOID LOS_BoadExampleTskfunc(VOID)
{
	float temp = 0;
	while (1)
    {
        LOS_EvbLedControl(LOS_LED3, LED_ON);
        LOS_TaskDelay(500);
        LOS_EvbLedControl(LOS_LED3, LED_OFF);
        LOS_TaskDelay(500);

        /* Send Data */
//        UDPsend("Microsemi Hyderabad");
    	temp = ADT7420_GetTemperature();
    	ftoa(temp,&rcv_data,4);
    	/* Send Data */
    	UDPsend(&rcv_data);


    }
}
void LOS_BoadExampleEntry(void)
{
    UINT32 uwRet;
    TSK_INIT_PARAM_S stTaskInitParam;

    (VOID)memset((void *)(&stTaskInitParam), 0, sizeof(TSK_INIT_PARAM_S));
    stTaskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)LOS_BoadExampleTskfunc;
    stTaskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE;
    stTaskInitParam.pcName = "BoardDemo";
    stTaskInitParam.usTaskPrio = 10;
    uwRet = LOS_TaskCreate(&g_uwboadTaskID, &stTaskInitParam);

    if (uwRet != LOS_OK)
    {
        return;
    }
    return;
}

/*****************************************************************************
Function    : main
Description : Main function entry
Input       : None
Output      : None
Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT
int main(void)
{
    UINT32 uwRet;
    /*
        add you hardware init code here
        for example flash, i2c , system clock ....
     */
    //HAL_init();....

    /*Init LiteOS kernel */
    uwRet = LOS_KernelInit();
    if (uwRet != LOS_OK) {
        return LOS_NOK;
    }
    /* Enable LiteOS system tick interrupt */
    LOS_EnableTick();

    LOS_EvbSetup();//init the device on the dev baord

    /*-------------------------------------------------------------------------
     * Initialize the I2C0 Driver
    */
    I2C_init(&g_core_i2c, COREI2C_BASE_ADDR, MASTER_SER_ADDR, I2C_PCLK_DIV_256);

    /* CoreI2C Master*/
    PLIC_SetPriority(External_27_IRQn, 1);
    PLIC_EnableIRQ(External_27_IRQn);

    /* Enable interrupts in general. */
    HAL_enable_interrupts();

    LOS_BoadExampleEntry();

    /* Kernel start to run */
    LOS_Start();
    for (;;);
    /* Replace the dots (...) with your own code. */
}
