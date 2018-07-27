#ifndef _LOS_BSP_KEY_H
#define _LOS_BSP_KEY_H

/*#define LOS_KEY_PRESS   0 for 150 kit
  #define LOS_KEY_PRESS   1 for Yellow board
 * */
#define LOS_KEY_PRESS   1

#ifdef LOS_M2S150_RV32

#define SWITCH1			GPIO_0
#define SWITCH2			GPIO_2
#define SWITCH3			GPIO_1
#define SWITCH4			GPIO_3

/*using SWITCH2 on 150 kit for SW2
  using SWITCH1 on Yellow board for SW1*/
#define USER_KEY        SWITCH1		//this used by los_inspect_entry.c

#endif

#define LOS_GPIO_ERR    0xFF

extern void LOS_EvbKeyInit(void);

unsigned int LOS_EvbGetKeyVal(int KeyNum);

#endif
