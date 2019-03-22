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
/* ============================ [ MACROS    ] ====================================================== */
/* ============================ [ TYPES     ] ====================================================== */
/* ============================ [ DECLARES  ] ====================================================== */
extern void OsTick(void);
/* ============================ [ DATAS     ] ====================================================== */
char FlashDriverRam[4096];
/* ============================ [ LOCALS    ] ====================================================== */
const Mcu_ConfigType const McuConfigData[1];
const Can_ConfigType Can_ControllerCfgData;
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

}

void BL_HwMonitor(void)
{

}

void application_main(void)
{

}


#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt 7 void Isr_SystemTick(void)
{
	CRGFLG &=0xEF;			// clear the interrupt flag
	OsTick();
}
#pragma CODE_SEG DEFAULT
