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
#include <assert.h>
#include <os/os.h>
#include "syscfg/syscfg.h"
#include "hal/hal_os_tick.h"
//#include <env/encoding.h>
#include <riscv_hal.h>
#include <hw_platform.h>

static uint64_t last_tick_time;
static uint32_t ticks_per_ostick;

#define RTC_FREQ        83000000UL
#define RTC_PRESCALER 	100

uint64_t get_timer_value(void);
void set_mtimecmp(uint64_t time);

void
os_tick_idle(os_time_t ticks)
{
}

void
os_tick_init(uint32_t os_ticks_per_sec, int prio)
{
    ticks_per_ostick = (RTC_FREQ / (RTC_PRESCALER*os_ticks_per_sec));
    last_tick_time = get_timer_value();
    set_mtimecmp(last_tick_time + ticks_per_ostick);

    set_csr(mie, MIP_MTIP);
}
volatile uint32_t test=0;
void
timer_interrupt_handler(void)
{

    uint64_t time = get_timer_value();
    int delta = (int)(time - last_tick_time);
    last_tick_time = time;
    test++;
    int ticks = delta / ticks_per_ostick;
    set_mtimecmp(time + ticks_per_ostick);

    os_time_advance(ticks);
}
