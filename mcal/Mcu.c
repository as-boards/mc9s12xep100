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
#include "Os.h"
/* ============================ [ MACROS    ] ====================================================== */
#define CPU_FREQUENCY 32000000
#ifdef USE_CLIB_STDIO_PRINTF
#define TERMIO_PutChar __putchar
#endif


#define LEDCPU     PORTK_PK4
#define LEDCPU_dir DDRK_DDRK4
/* ============================ [ TYPES     ] ====================================================== */
typedef struct {
	uint16 tag;   /* 2 bytes */
	FP     entry; /* 3 bytes */
	uint8  reserved;
} AppTag_Type;
/* ============================ [ DECLARES  ] ====================================================== */
extern void OsTick(void);

#if !defined(__AS_BOOTLOADER__)
extern const FP tisr_pc[];
#endif

#ifdef USE_ASKAR
extern void EnterISR(void);
extern void LeaveISR(void);
#endif

extern void _Startup(void);
/* ============================ [ DATAS     ] ====================================================== */
const Mcu_ConfigType const McuConfigData[1];
#if !defined(__AS_BOOTLOADER__)
#pragma DATA_SEG __NEAR_SEG APP_TAG
const AppTag_Type app_tag = { 0xA55A, _Startup, 0x5A };
#pragma DATA_SEG __NEAR_SEG DEFAULT
#endif
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

#if !defined(__AS_BOOTLOADER__)
	{
		volatile uint16 tag;
		tag = app_tag.tag; /* access to keep the app tag */
	}
#endif
#if !defined(__AS_BOOTLOADER__)
	IVBR = ((uint16)tisr_pc)>>8;
#endif
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
	COPCTL = 0x41;			/* enable the COP */
	while(1);
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

	LEDCPU_dir=1;
}

void TaskIdleHook(void)
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

/*
#python code to search the best for the given expect period
OSCLK = 16*1000000
expect = 1/100
diff = OSCLK
best = None
# RTDEC = 0
for rtr30 in range(16):
  for rtr64 in range(8):
    period = (rtr30+1)*2**(rtr64+9)/OSCLK
    ndiff = abs(period-expect)
    if(ndiff < diff):
      diff = ndiff
      best = (0, rtr64, rtr30, period, diff)
# RTDEC = 1
TABLE = [1,2,5,10,20,50,100,200]
for rtr30 in range(16):
  for rtr64 in range(8):
    period = (rtr30+1)*TABLE[rtr64]*1000/OSCLK
    ndiff = abs(period-expect)
    if(ndiff < diff):
      diff = ndiff
      best = (1, rtr64, rtr30, period, diff)
print('best is', hex((best[0]<<7)+(best[1]<<4)+best[2]))
 */
void StartOsTick(void)
{
#if OS_TICKS_PER_SECOND == 100
	RTICTL = 0xc7;       /* period is 10ms */
#else
#error wrong configuration of OS_TICKS_PER_SECOND
#endif
	CRGINT_RTIE=1;       /* enable real-time interrupt */
}

#pragma CODE_SEG __NEAR_SEG NON_BANKED
#if 1 //defined(__AS_BOOTLOADER__)
#define ISRNO_VRTI      VectorNumber_Vrti
#define ISRNO_VCAN0RX   VectorNumber_Vcan0rx
#define ISRNO_VCAN0TX   VectorNumber_Vcan0tx
#define ISRNO_VCAN0ERR  VectorNumber_Vcan0err
#define ISRNO_VCAN0WKUP VectorNumber_Vcan0wkup
#define ISRNO_VSWI      VectorNumber_Vswi
#else
#define ISRNO_VRTI
#define ISRNO_VCAN0RX
#define ISRNO_VCAN0TX
#define ISRNO_VCAN0ERR
#define ISRNO_VCAN0WKUP
#define ISRNO_VSWI
#endif
interrupt ISRNO_VRTI void Isr_SystemTick(void)
{
	#ifdef USE_ASKAR
	EnterISR();
	#endif
	CRGFLG &=0xEF;			// clear the interrupt flag
	OsTick();
	#ifdef USE_ASKAR
	SignalCounter(0);
	#endif
	#ifdef USE_ASKAR
	LeaveISR();
	#endif
}
#ifdef USE_CAN
interrupt ISRNO_VCAN0RX  void Can_0_RxIsr_Entry( void  ) {	Can_0_RxIsr(); }
interrupt ISRNO_VCAN0TX  void Can_0_TxIsr_Entry( void  ) {	Can_0_TxIsr(); }
interrupt ISRNO_VCAN0ERR void Can_0_ErrIsr_Entry( void  ) {	Can_0_ErrIsr(); }
interrupt ISRNO_VCAN0WKUP void Can_0_WakeIsr_Entry( void  ) {	Can_0_WakeIsr(); }
#endif

#if !defined(USE_ASKAR)
interrupt ISRNO_VSWI void Isr_SoftwareInterrupt(void)
{

}
#endif
#pragma CODE_SEG DEFAULT
