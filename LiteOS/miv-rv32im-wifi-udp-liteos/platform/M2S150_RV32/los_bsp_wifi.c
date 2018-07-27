#include "los_bsp_uart.h"
#include "los_bsp_led.h"
#include <stdarg.h>
#include <stdio.h>
#include "riscv_hal.h"
#include "hal.h"

/******************************************************************************
 * CoreUARTapb instance data.
 *****************************************************************************/
#ifdef LOS_M2S150_RV32

#include "core_uart_apb.h"
#include "hw_platform.h"

//const char SSID[] = "Connectify-me";
//const char password[] = "fewepom4";

const char SSID[] = "NITINL";
const char password[] = "12345678";

const char port[] = "8080";
const char msg_hdr[] = "+IPD";

const uint8_t opt_msg[] =
"\r\n\r\n\
******************************************************************************\r\n\
************* RISC-V WiFi3 UDP Example Project with LiteOS **************\r\n\
******************************************************************************\r\n\
  WiFI connection is now established. UDP packet is being sent every after every 1 Sec.\r\n";


// responses to parse
const uint8_t OK = 1;
const uint8_t ERROR = 2;
const uint8_t NO_CHANGE = 3;
const uint8_t FAIL = 4;
const uint8_t READY = 5;

uint8_t CR = 0x0D;
uint8_t LF = 0x0A;
uint8_t SQ = 0x22;
uint8_t SC = 0x2C;

/*WiFi over UART communication data*/
UART_instance_t wifi_uart;
uint8_t linefreed = 0;
uint8_t tmp = 0;
uint8_t rx_data[64] = {0x00};
volatile uint16_t rx_cnt = 0;
char state;
char response_rcvd;
char responseID, response = 0;

uint8_t rx_buff[3000];
uint8_t data_ready;
uint8_t tmp;
uint16_t data_len = 0;
uint8_t received_data[16], ip_address[16];

uint8_t updbuf[16] = {0};

volatile uint8_t cmd_size = 0;
uint8_t tx_data[500] = {0x00};
uint16_t tx_cnt = 0;
static uint8_t i = 0;


void calculate_cmd_size(uint8_t *CMD)
{
	uint8_t *ptr = CMD;

	while(*ptr)
	{
		ptr++;
		cmd_size++;
	}
}

void UART_Write_AT(uint8_t *CMD)
{
   uint8_t data = 0;

   data_len = 0;
   calculate_cmd_size(CMD);
   UART_polled_tx_string(&wifi_uart, CMD);
   data = 0x0D;
   UART_send(&wifi_uart, &data, 1);
   data = 0x0A;
   UART_send(&wifi_uart, &data, 1);
}

void reset_buff()
{
    memset(rx_buff,0,sizeof(rx_buff));
    data_ready = 0;
    data_len = 0;
    cmd_size = 0;
}

/* Read the response from WiFi3 Click using UART polled method. */
uint8_t read_response(void)
{
    uint8_t res = 0;
    uint8_t rx_byte = 0x00;
    uint8_t rx_size = 0x00;

	do
	{
		rx_size = UART_get_rx( &wifi_uart, &rx_byte, 1);
		while ( rx_size > 0 )
		{
			rx_buff[data_len] = rx_byte;
			if(rx_buff[data_len - 1] == 'O')
			{
				if(rx_buff[data_len] == 'K')
				{
				   res = 1;
				}
			}

			data_len++;
			rx_size = 0;
		}

		if(res == 1)
		{
			res = 0;
			LOS_EvbUartWriteStr(&rx_buff);
			break;
		}
	}while(data_len < sizeof(rx_buff));

	return res;
}

void UDPsend(uint8_t* buffer)
{
    uint8_t res = 0;
    uint8_t i = 0;
    uint8_t rx_byte = 0x00;
    uint8_t rx_size = 0x00;
    uint8_t data_send = 0;

	UART_rxflush(&wifi_uart);
	UART_polled_tx_string(&wifi_uart,"AT+CIPSEND=70");
    UART_send(&wifi_uart,&CR,1);
    UART_send(&wifi_uart,&LF,1);
    do
	{
        rx_size = UART_get_rx( &wifi_uart, &rx_byte, 1 );
		while ( rx_size > 0 )
		{
			rx_buff[data_len] = rx_byte;
			if(rx_buff[data_len] == '>')
			{
				UART_polled_tx_string(&wifi_uart,"Microsemi Temperature sensor ADT7420(in degrees Celsius):");
				while(data_send != 1)
				{
					UART_send(&wifi_uart,&buffer[i],1);
					i++;
					if(i == 20)
						data_send = 1;
				}

			}

		    if((rx_buff[data_len - 1] == 'O') && (rx_buff[data_len] == 'K') && (data_send == 1))
		    {
		    	res = 1;
		    }

		 	data_len++;
		 	rx_size = 0;
	   }

	   if(res == 1)
	   {
	       res = 0;
	       data_len = 0;
		   break;
	   }

    } while(data_len < sizeof(rx_buff));
}


#endif /*LOS_M2S150_RV32*/

void delay_ms(uint32_t temp)
{
	volatile uint32_t count = 0;
	for(count = 0; count < (temp*2200); count++);
}

void LOS_EvbwifiInit(void)
{
#ifdef LOS_M2S150_RV32

	int i = 0;
    uint8_t rx_byte = 0x00;
    size_t rx_size = 0x00;
    uint8_t res = 0;
    uint8_t data_send = 0;

    PLIC_init();

    UART_init(&wifi_uart,
              COREUARTAPB1_BASE_ADDR,
			  BAUD_VALUE_115200,
              (DATA_8_BITS | NO_PARITY));

    LOS_EvbLedControl(LOS_LED1, LED_OFF);
    LOS_EvbLedControl(LOS_LED2, LED_ON);
    delay_ms(6000);

    /* Resetting module. */
    UART_rxflush(&wifi_uart);
    reset_buff();
	UART_Write_AT("AT+RST");
	response  = read_response();
    if(!response)
    {
    	LOS_EvbUartWriteStr("\r\n RESET command successful. \r\n ");
    }
    else
    {
    	LOS_EvbUartWriteStr("\r\n RESET command failed. \r\n ");
    }

	delay_ms(1500);

    /* Set maximum value of RF TX Power. */
	UART_rxflush(&wifi_uart);
	reset_buff();
	UART_Write_AT("AT+RFPOWER=50");
	response  = read_response();
    if(!response)
    {
    	LOS_EvbUartWriteStr("\r\n RFPOWER command successful. \r\n ");
    }
    else
    {
    	LOS_EvbUartWriteStr("\r\n RFPOWER command failed. \r\n ");
    }
	delay_ms(1500);

	/* Change the working mode to 1. */
	UART_rxflush(&wifi_uart);
	reset_buff();
	UART_Write_AT("AT+CWMODE_CUR=1");
	response  = read_response();
    if(!response)
    {
    	LOS_EvbUartWriteStr("\r\n CWMODE_CUR command successful. \r\n ");
    }
    else
    {
    	LOS_EvbUartWriteStr("\r\n CWMODE_CUR command failed. \r\n ");
    }
	delay_ms(1500);

	/* Setting connection mode. */
	reset_buff();
	UART_rxflush(&wifi_uart);
	UART_Write_AT("AT+CIPMUX=1");
	response  = read_response();
    if(!response)
    {
    	LOS_EvbUartWriteStr("\r\n CIPMUX command successful. \r\n ");
    }
    else
    {
    	LOS_EvbUartWriteStr("\r\n CIPMUX command  FAILED. \r\n ");
    }
	delay_ms(1500);


    reset_buff();
    UART_rxflush(&wifi_uart);
    /* Connecting to AP. */
    cmd_size = sizeof("AT+CWJAP_CUR=");
    cmd_size += sizeof(SSID);
    cmd_size += sizeof(password);
    cmd_size += 2;

    UART_polled_tx_string(&wifi_uart,"AT+CWJAP_CUR=");
    UART_send(&wifi_uart,&SQ,1);
    UART_polled_tx_string(&wifi_uart,SSID);
    UART_send(&wifi_uart,&SQ,1);
    UART_send(&wifi_uart,&SC,1);
    UART_send(&wifi_uart,&SQ,1);
    UART_polled_tx_string(&wifi_uart,password);
    UART_send(&wifi_uart,&SQ,1);
    UART_send(&wifi_uart,&CR,1);
    UART_send(&wifi_uart,&LF,1);
    response  = read_response();
    if(!response)
    {
    	LOS_EvbUartWriteStr("\r\n CWJAP_CUR command successful. \r\n\r\n");
    }
    else
    {
    	LOS_EvbUartWriteStr("\r\n CWJAP_CUR command failed. \r\n\r\n");
    }
    delay_ms(1500);

    /* Check the assigned IP value. */
    reset_buff();
	UART_rxflush(&wifi_uart);
	UART_Write_AT("AT+CWJAP_CUR?");
	response  = read_response();
	if(!response)
	{
		LOS_EvbUartWriteStr("\r\n CWJAP_CUR? command successful. \r\n\n\n");
	}
    else
    {
    	LOS_EvbUartWriteStr("\r\n CWJAP_CUR? command failed. \r\n\r\n");
    }
	delay_ms(1500);

    /* Check the assigned IP value. */
    reset_buff();
	UART_rxflush(&wifi_uart);
	UART_Write_AT("AT+CIFSR");
	response = read_response();
	if(!response)
	{
		LOS_EvbUartWriteStr("\r\n CIFSR command successful. \r\n\n\n");
	}
    else
    {
    	LOS_EvbUartWriteStr("\r\n CIFSR command fail. \r\n\r\n");
    }
	delay_ms(1500);

	//set single connection model
    reset_buff();
	UART_rxflush(&wifi_uart);
	UART_Write_AT("AT+CIPMUX=0");
	response = read_response();
	if(!response)
	{
		LOS_EvbUartWriteStr("\r\n AT+CIPMUX ok. \r\n\n\n");
	}
    else
    {
    	LOS_EvbUartWriteStr("\r\n AT+CIPMUX fail. \r\n\r\n");
    }
	delay_ms(500);

	//create a udp client
    reset_buff();
	UART_rxflush(&wifi_uart);
	UART_Write_AT("AT+CIPSTART=\"UDP\",\"192.168.137.1\",8888,6000,0");
	//UART_Write_AT("AT+CIPSTART=\"UDP\",\"10.60.244.247\",8888,6000,0");
	//UART_Write_AT("AT+CIPSTART=\"UDP\",\"169.254.102.231\",8888,6000,0");
	response = read_response();
	if(!response)
	{
		LOS_EvbUartWriteStr("\r\n create udp client ok. \r\n\n\n");
	}
    else
    {
    	LOS_EvbUartWriteStr("\r\n create udp client fail. \r\n\r\n");
    }
	delay_ms(500);

	/* Get inputs from user to determine which operation to perform */
	LOS_EvbUartWriteStr(opt_msg);

#endif /*LOS_M2S150_RV32*/

}

