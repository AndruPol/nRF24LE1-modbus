/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2010 Christian Walter <cwalter@embedded-solutions.at>
 *
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * IF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id: porttimer.c,v 1.1 2010/06/06 13:07:20 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include "interrupt.h"
#include "gpio.h"
#include "timer0.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbconfig.h"
#include "mbport.h"
#include "port.h"

/* ----------------------- Defines ------------------------------------------*/

#define TM50USTICKS			52		// ~50uS - 52

/* ----------------------- Static variables ---------------------------------*/
static USHORT timerout = 0;

/* ----------------------- Start implementation -----------------------------*/
#if MB_RTU_ENABLED > 0
interrupt_isr_t0() {
	pxMBPortCBTimerExpired( );
}

BOOL xMBPortTimersInit( USHORT usTim1Timerout50us ) {

	timerout = 0xFFFF - (usTim1Timerout50us * TM50USTICKS);

	timer0_configure(
		TIMER0_CONFIG_OPTION_MODE_1_16_BIT_CTR_TMR
		| TIMER0_CONFIG_OPTION_FUNCTION_TIMER
		| TIMER0_CONFIG_OPTION_GATE_ALWAYS_RUN_TIMER,
		timerout
	);

	interrupt_control_t0_enable();

	return TRUE;
}

void vMBPortTimersEnable(  ) {
	timer0_stop();
	timer0_set_t0_val(timerout);
	timer0_run();
}

void vMBPortTimersDisable(  ) {
	timer0_stop();
}

void vMBPortTimerClose( void ) {
	timer0_stop();
}
#endif
