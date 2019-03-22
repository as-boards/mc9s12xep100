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
#include "Can.h"
/* ============================ [ MACROS    ] ====================================================== */
/* ============================ [ TYPES     ] ====================================================== */
/* ============================ [ DECLARES  ] ====================================================== */
/* ============================ [ DATAS     ] ====================================================== */
/* ============================ [ LOCALS    ] ====================================================== */
/* ============================ [ FUNCTIONS ] ====================================================== */
void Can_Init( const Can_ConfigType *Config )
{
}

void Can_DeInit(void)
{
}


void Can_InitController( uint8 controller, const Can_ControllerConfigType *config)
{
}

Can_ReturnType Can_SetControllerMode( uint8 Controller, Can_StateTransitionType transition )
{
}

void Can_DisableControllerInterrupts( uint8 controller )
{
}

void Can_EnableControllerInterrupts( uint8 controller )
{
}

Can_ReturnType Can_Write( Can_Arc_HTHType hth, Can_PduType *pduInfo )
{
}


void Can_Cbk_CheckWakeup( uint8 controller )
{
}

void Can_MainFunction_Write( void )
{
}

void Can_MainFunction_Read( void )
{
}

void Can_MainFunction_BusOff( void )
{
}

void Can_MainFunction_Error( void )
{
}

void Can_MainFunction_Wakeup( void )
{
}

