/**
 * AS - the open source Automotive Software on https://github.com/parai
 *
 * Copyright (C) 2019  AS <parai@foxmail.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
/* ============================ [ INCLUDES  ] ====================================================== */
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "miniblt.h"
#include "Mcu.h"
#include "Can.h"
#include "Os.h"
#ifdef USE_SHELL
#include "shell.h"
#endif

#include "Flash.h"
/* ============================ [ MACROS    ] ====================================================== */
#define LEDCPU     PORTK_PK4
#define LEDCPU_dir DDRK_DDRK4

#define OSCCLK 16000000
/* ============================ [ TYPES     ] ====================================================== */
/* ============================ [ DECLARES  ] ====================================================== */
extern TickType OsTickCounter;
/* ============================ [ DATAS     ] ====================================================== */
#pragma DATA_SEG __NEAR_SEG FLASH_RAM
tFlashHeader FlashDriverRam @ 0x3900;
#pragma DATA_SEG DEFAULT
/* ============================ [ LOCALS    ] ====================================================== */
/* ============================ [ FUNCTIONS ] ====================================================== */
void BL_HwInit(void)
{
	LEDCPU_dir=1;
}

void BL_HwMonitor(void)
{
	static TickType timer = 0;
	if((OsTickCounter-timer) >= (OS_TICKS_PER_SECOND/2))
	{
		LEDCPU=~LEDCPU;
		timer = OsTickCounter;
	}
	#ifdef USE_SHELL
	if(SCI0SR1_RDRF)
	{
		char ch = SCI0DRL;
		if('\r' == ch) ch = '\n';
		SHELL_input(ch);
	}
	#endif
}

void application_main(void)
{

}
