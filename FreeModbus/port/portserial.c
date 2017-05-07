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
 * File: $Id: portserial.c,v 1.1 2010/06/06 13:07:20 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "uart.h"
#include "interrupt.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbconfig.h"
#include "mbport.h"
#include "port.h"

/* ----------------------- Defines ------------------------------------------*/
#define USART_INVALID_PORT      ( 0xFF )
#define USART_NOT_RE_IDX        ( 3 )
#define USART_DE_IDX            ( 4 )
#define USART_IDX_LAST          ( 1 ) // only one usart

#define	BOARD_SERIAL_INPUT		GPIO_PIN_CONFIG_OPTION_DIR_INPUT | GPIO_PIN_CONFIG_OPTION_PIN_MODE_INPUT_BUFFER_ON_NO_RESISTORS
#define	BOARD_SERIAL_OUTPUT		GPIO_PIN_CONFIG_OPTION_DIR_OUTPUT | GPIO_PIN_CONFIG_OPTION_OUTPUT_VAL_SET | GPIO_PIN_CONFIG_OPTION_PIN_MODE_OUTPUT_BUFFER_NORMAL_DRIVE_STRENGTH

#define BOARD_SERIAL_TX			GPIO_PIN_ID_P0_3		// P0.3
#define BOARD_SERIAL_RX			GPIO_PIN_ID_P0_4		// P0.4
#define BOARD_MAX485_TX_EN		GPIO_PIN_ID_P1_2		// P1.2
#define BOARD_MAX485			1						// MAX485 TX enable control

#define BOARD_UART8BIT			( UART_CONFIG_OPTION_ENABLE_RX | \
								  UART_CONFIG_OPTION_MODE_1_UART_8_BIT | \
								  UART_CONFIG_OPTION_CLOCK_FOR_MODES_1_3_USE_BR_GEN | \
								  UART_CONFIG_OPTION_BIT_SMOD_SET )
#define BOARD_UART9BIT			( UART_CONFIG_OPTION_ENABLE_RX | \
								  UART_CONFIG_OPTION_MODE_3_UART_9_BIT | \
								  UART_CONFIG_OPTION_CLOCK_FOR_MODES_1_3_USE_BR_GEN | \
								  UART_CONFIG_OPTION_BIT_SMOD_SET )
#define BOARD_SPD2400			816
#define BOARD_SPD4800			920
#define BOARD_SPD9600			972
#define BOARD_SPD19200			998
#define BOARD_SPD38400			1011

/* ----------------------- Static variables ---------------------------------*/

static UCHAR    ucUsedPort = USART_INVALID_PORT;
static eMBParity parity;
static volatile BOOL	rxOK;
static volatile UCHAR	ucRX;

static inline BOOL parityGet(UCHAR x) {
	x ^= x >> 4;
	x ^= x >> 2;
	x ^= x >> 1;

	if (parity == MB_PAR_ODD) {
		return !(x & 1);
	} else if (parity == MB_PAR_EVEN) {
		return (x & 1);
	}
	return 0;
}

#if MB_RTU_ENABLED > 0

interrupt_isr_uart() {
	if (interrupt_is_flag_active_uart_rx()) {
		if (uart_rx_data_ready()) {
			ucRX = (UCHAR) uart_get();
			if (parity != MB_PAR_NONE) {
				rxOK = uart_get_rx_bit_8() == parityGet(ucRX);
			}
			pxMBFrameCBByteReceived( );
		}
		interrupt_clear_uart_rx();
	}
	if (interrupt_is_flag_active_uart_tx()) {
		if (uart_tx_data_sent()) {
			pxMBFrameCBTransmitterEmpty( );
		}
		interrupt_clear_uart_tx();
	}
}

BOOL xMBPortSerialPutByte( CHAR ucByte ) {
	if (parity != MB_PAR_NONE) {
		if (parityGet(ucByte)) {
			uart_set_tx_bit_8();
		} else {
			uart_clear_tx_bit_8();
		}
	}
	uart_send(ucByte);
	return TRUE;
}

BOOL xMBPortSerialGetByte( CHAR * pucByte ) {
	*pucByte = ucRX;
	return rxOK;
}

void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable ) {
	if( xRxEnable )  {
#if BOARD_MAX485
		delay_us(100);
		gpio_pin_val_clear(BOARD_MAX485_TX_EN);
#endif
	}

	if( xTxEnable )  {
#if BOARD_MAX485
		delay_us(100);
		gpio_pin_val_set(BOARD_MAX485_TX_EN);
#endif
		pxMBFrameCBTransmitterEmpty( );
	}
}

BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity ) {
	BOOL bStatus = TRUE;
	UCHAR uartCfg;

	if (ucPORT > USART_IDX_LAST) return FALSE;

	rxOK = TRUE;
	parity = eParity;
	uartCfg = BOARD_UART8BIT;
	if ( parity != MB_PAR_NONE ) {
		uartCfg = BOARD_UART9BIT;
	}

    if ( ucDataBits != 8 )  {
      return FALSE;
    }

#if BOARD_MAX485
	gpio_pin_configure(BOARD_MAX485_TX_EN, BOARD_SERIAL_OUTPUT);
	delay_ms(1);
#endif

	gpio_pin_configure(BOARD_SERIAL_RX, BOARD_SERIAL_INPUT);
	gpio_pin_configure(BOARD_SERIAL_TX, BOARD_SERIAL_OUTPUT);

    switch (ulBaudRate) {
    case 2400:
    	uart_configure_manual_baud_calc(uartCfg, BOARD_SPD2400);
    	break;
    case 4800:
    	uart_configure_manual_baud_calc(uartCfg, BOARD_SPD4800);
    	break;
	case 9600:
    	uart_configure_manual_baud_calc(uartCfg, BOARD_SPD9600);
		break;
	case 19200:
    	uart_configure_manual_baud_calc(uartCfg, BOARD_SPD19200);
		break;
	case 38400:
    	uart_configure_manual_baud_calc(uartCfg, BOARD_SPD38400);
		break;
    default:
    	bStatus = FALSE;
    	break;
    }

	interrupt_clear_uart_rx();
	interrupt_clear_uart_tx();

	interrupt_control_uart_enable();

#if BOARD_MAX485
	gpio_pin_val_clear(BOARD_MAX485_TX_EN);
#endif

	ucUsedPort = ucPORT;
    return bStatus;
}

void vMBPortSerialClose( void ) {
	if (ucUsedPort != USART_INVALID_PORT)   {
#if BOARD_MAX485
		gpio_pin_val_clear(BOARD_MAX485_TX_EN);
#endif
		uart_rx_disable();
		ucUsedPort = USART_INVALID_PORT;
	}
}
#endif
