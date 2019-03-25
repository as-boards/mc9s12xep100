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

#define APP_TAG_ADDR 0xC000
/* ============================ [ TYPES     ] ====================================================== */
typedef struct {
	uint16 tag;   /* 2 bytes */
	FP     entry; /* 3 bytes */
	uint8  reserved;
} AppTag_Type;
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
}

void application_main(void)
{
	AppTag_Type *appTag = (AppTag_Type*)APP_TAG_ADDR;

	if( (0xA55A == appTag->tag) &&
		(0x5A == appTag->reserved) &&
		(NULL != appTag->entry))
	{
		printf("jmp %X\n", (uint32)appTag->entry);
		CRGINT_RTIE = 0;	/* stop os tick ISR */
		appTag->entry();
	}
}
