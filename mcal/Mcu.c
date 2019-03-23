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
#include "Mcu.h"
#include <hidef.h>
#include "derivative.h"
/* ============================ [ MACROS    ] ====================================================== */
#define CPU_FREQUENCY 32000000
#ifdef USE_CLIB_STDIO_PRINTF
#define TERMIO_PutChar __putchar
#endif
/* ============================ [ TYPES     ] ====================================================== */
/* ============================ [ DECLARES  ] ====================================================== */
/* ============================ [ DATAS     ] ====================================================== */
/* ============================ [ LOCALS    ] ====================================================== */
/* ============================ [ FUNCTIONS ] ====================================================== */
void TERMIO_PutChar(char c)
{
	while(!SCI0SR1_TDRE);
	SCI0DRL = c;
}

void Mcu_Init(const Mcu_ConfigType *configPtr)
{
	(void*)configPtr;
	CLKSEL &= 0x7f;       //set OSCCLK as sysclk
	PLLCTL &= 0x8F;       //Disable PLL circuit
	CRGINT &= 0xDF;

	#if(CPU_FREQUENCY == 40000000)
	SYNR = 0x44;
	#elif(CPU_FREQUENCY == 32000000)
	SYNR = 0x43;
	#elif(CPU_FREQUENCY == 24000000)
	SYNR = 0x42;
	#endif

	REFDV = 0x81;
	PLLCTL =PLLCTL|0x70;  //Enable PLL circuit

}

void Mcu_SetMode( Mcu_ModeType McuMode )
{
	(void)McuMode;
}


uint32_t McuE_GetSystemClock(void)
{
	return (uint32_t)CPU_FREQUENCY;
}

Std_ReturnType Mcu_InitClock( const Mcu_ClockType ClockSetting )
{
	(void) ClockSetting;
	return E_OK;
}

void Mcu_PerformReset( void )
{

}

Mcu_PllStatusType Mcu_GetPllStatus( void )
{
	Mcu_PllStatusType status = MCU_PLL_UNLOCKED;

	if(0 == (CRGFLG&0x08))
	{
		/* do nothing */
	}
	else
	{
		status = MCU_PLL_LOCKED;
	}

	return status;
}


Mcu_ResetType Mcu_GetResetReason( void )
{
	return MCU_POWER_ON_RESET;
}

void Mcu_DistributePllClock( void )
{
	CLKSEL |= 0x80; /* set PLLCLK as sysclk */
	/* Init UART for debug */
	SCI0BD = CPU_FREQUENCY/16/115200;
	SCI0CR1 = 0x00;
	SCI0CR2 = 0x0C;	/* enable RX and TX, no interrupt */
}


imask_t __Irq_Save(void)
{
	asm psha;
	asm tpa;
	asm tab;
	asm sei;
	asm pula;
}

void Irq_Restore(imask_t mask)
{
	(void)mask;
	asm psha;
	asm tba;
	asm tap;
	asm pula;
}

void Irq_Enable(void)
{
	asm cli;
}

void Irq_Disable(void)
{
	asm sei;
}
