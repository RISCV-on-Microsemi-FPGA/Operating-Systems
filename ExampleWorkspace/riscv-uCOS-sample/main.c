/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                              uC/OS-II
*                                            EXAMPLE CODE
*
* Filename : main.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_mem.h>
#include  <os.h>
#include  <bsp_os.h>
#include  <bsp_led.h>

#include  "app_cfg.h"
#include "core_uart_apb.h"
#include "hw_platform.h"
/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define MUTEX_PRIORITY 	5u

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK  StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE];
static  OS_STK  AppTask1_Stk[APP_CFG_TASK_STK_SIZE];
static  OS_STK  AppTask2_Stk[APP_CFG_TASK_STK_SIZE];

UART_instance_t g_uart;

uint8_t g_message[] = "\n\ruCOS Example \n\r\n\r";
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  StartupTask (void  *p_arg);
static  void  AppTask1 (void  *p_arg);
static  void  AppTask2 (void  *p_arg);

static  void  AppTaskCreate(void);
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

int  main (void)
{
#if OS_TASK_NAME_EN > 0u
    CPU_INT08U  os_err;
#endif

    UART_init(&g_uart,
              COREUARTAPB0_BASE_ADDR,
              BAUD_VALUE_115200,
              (DATA_8_BITS | NO_PARITY) );  /* Initialize uart for messages */
              
    UART_send( &g_uart, g_message, sizeof(g_message) );

    BSP_OS_TickInit();                                          /* Initialize kernel tick timer                         */

    Mem_Init();                                                 /* Initialize Memory Managment Module                   */
    CPU_IntDis();                                               /* Disable all Interrupts                               */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */


    OSInit();                                                   /* Initialize uC/OS-II                                  */

    OSMutexCreate(MUTEX_PRIORITY, &os_err);

    OSTaskCreateExt( StartupTask,                               /* Create the startup task                              */
                     0,
                    &StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE - 1u],
                    APP_CFG_STARTUP_TASK_PRIO,
                    APP_CFG_STARTUP_TASK_PRIO,
                    &StartupTaskStk[0u],
                    APP_CFG_STARTUP_TASK_STK_SIZE,
                     0u,
                     (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if OS_TASK_NAME_EN > 0u
    OSTaskNameSet( APP_CFG_STARTUP_TASK_PRIO,
                   (INT8U *)"Startup Task",
                           &os_err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */

    while (DEF_ON) {                                            /* Should Never Get Here.                               */
        ;
    }
}


/*
*********************************************************************************************************
*                                            STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'StartupTask()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  StartupTask (void *p_arg)
{
   (void)p_arg;


    OS_TRACE_INIT();                                            /* Initialize the uC/OS-II Trace recorder               */

    BSP_OS_TickEnable();                                        /* Enable the tick timer and interrupt                  */

#if (OS_TASK_STAT_EN > 0u)
    OSStatInit();                                               /* Determine CPU capacity                               */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    BSP_LED_Init();

    AppTaskCreate();

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
        BSP_LED_Toggle(LED1_RED);
        BSP_LED_Toggle(LED1_GREEN);
        OSTimeDlyHMSM(0u, 0u, 1u, 0u);
        UART_polled_tx_string( &g_uart, (const uint8_t *)"StartupTask :\r\n" );
    }
}
/*********************************************************************************************************/
static void AppTaskCreate(void)
{

    INT8U os_err;

    OSTaskCreateExt( AppTask1,                               /* Create the startup task                              */
                     0,
                    &AppTask1_Stk[APP_CFG_TASK_STK_SIZE - 1u],
                    APP_CFG_TASK1_PRIO,
                    APP_CFG_TASK1_PRIO,
                    &AppTask1_Stk[0u],
                    APP_CFG_TASK_STK_SIZE,
                     0u,
                    (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
    OSTaskCreateExt( AppTask2,                               /* Create the startup task                              */
                     0,
                    &AppTask2_Stk[APP_CFG_TASK_STK_SIZE - 1u],
                    APP_CFG_TASK2_PRIO,
                    APP_CFG_TASK2_PRIO,
                    &AppTask2_Stk[0u],
                    APP_CFG_TASK_STK_SIZE,
                     0u,
                    (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if OS_TASK_NAME_EN > 0u
    OSTaskNameSet(         APP_CFG_TASK1_PRIO,
                  (INT8U *)"AppTask1",
                           &os_err);
    OSTaskNameSet(         APP_CFG_TASK2_PRIO,
                  (INT8U *)"AppTask2",
                           &os_err);
#endif
}
/*********************************************************************************************************/

static  void  AppTask1 (void *p_arg)
{
   (void)p_arg;

   while(DEF_TRUE)
   {

        UART_polled_tx_string( &g_uart, (const uint8_t *)"Task - 1\r\n" );
        OSTimeDly(1000);
   }
}
/*********************************************************************************************************/

static  void  AppTask2 (void *p_arg)
{
   (void)p_arg;

   while(DEF_TRUE)
   {
        UART_polled_tx_string( &g_uart, (const uint8_t *)"Task - 2\r\n" );
        OSTimeDly(1000);
   }
}
/*********************************************************************************************************/

