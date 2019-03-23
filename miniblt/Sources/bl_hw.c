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
/* ============================ [ MACROS    ] ====================================================== */
#define LEDCPU     PORTK_PK4
#define LEDCPU_dir DDRK_DDRK4
/* ============================ [ TYPES     ] ====================================================== */
/* ============================ [ DECLARES  ] ====================================================== */
extern void OsTick(void);
extern TickType OsTickCounter;
/* ============================ [ DATAS     ] ====================================================== */
char FlashDriverRam[4096];
/* ============================ [ LOCALS    ] ====================================================== */
const Mcu_ConfigType const McuConfigData[1];
/* ============================ [ FUNCTIONS ] ====================================================== */

void StartOsTick(void)
{
	CRGINT_RTIE=1;       //enable real-time interrupt
	RTICTL = 0x70;       //period is 4.096ms
	//OSCCLK = 16 x 10E6
	//RTI ISR period =  1/OSCCLK * (0+1) * 2E(7+9)=0.004096s=4.096ms
}

void BL_HwInit(void)
{
	LEDCPU_dir=1;
}

void BL_HwMonitor(void)
{
	static TickType timer = 0;
	if((OsTickCounter-timer) >= 125)
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


#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt VectorNumber_Vrti void Isr_SystemTick(void)
{
	CRGFLG &=0xEF;			// clear the interrupt flag
	OsTick();
}

interrupt VectorNumber_Vcan0rx  void Can_0_RxIsr_Entry( void  ) {	Can_0_RxIsr(); }
interrupt VectorNumber_Vcan0tx  void Can_0_TxIsr_Entry( void  ) {	Can_0_TxIsr(); }
interrupt VectorNumber_Vcan0err void Can_0_ErrIsr_Entry( void  ) {	Can_0_ErrIsr(); }
interrupt VectorNumber_Vcan0wkup void Can_0_WakeIsr_Entry( void  ) {	Can_0_WakeIsr(); }
#pragma CODE_SEG DEFAULT
