/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef __M2S150_BSP_H
#define __M2S150_BSP_H

#include <stdint.h>
#include <DirectCore_Drivers/CoreGPIO/core_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define M2S150_UART0_RX             (0)
#define M2S150_UART0_TX             (1)

/* LED pins */
#define M2S150_LED0_PIN              (GPIO_0)
#define M2S150_LED1_PIN              (GPIO_1)
#define M2S150_LED2_PIN              (GPIO_2)
#define M2S150_LED3_PIN              (GPIO_3)

/*On SF2 150 Adv dev kit M2S150_LED2_PIN corresponds to LED DS2 on board*/
#define LED_BLINK_PIN               (M2S150_LED2_PIN)

#define CONSOLE_UART                "uart0"

extern uint8_t _ram_start;
#define RAM_SIZE                    0x8000

#ifdef __cplusplus
}
#endif

#endif  /* __M2S150_BSP_H */
