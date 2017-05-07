/*
 *  Created on: 15.04.2017
 *      Author: andru
 *
 *      nRF24LE1 FreeModbus port
 *
 *		based on great nRF24LE1 SDK https://github.com/DeanCording/nRF24LE1_SDK
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gpio.h"
#include "delay.h"
#include "interrupt.h"

#include "main.h"
#include "modbus_slave.h"
#include "mb.h"

#if EN_LED
#define LEDPIN	GPIO_PIN_ID_P1_4		// P1.4 - LED
#endif

// halt
void halt(void) {
	while (1) {
#if EN_LED
		gpio_pin_val_complement(LEDPIN);
		delay_ms(250);
#endif
	}
}

// T0 interrupt in porttimer.c
interrupt_isr_t0();

// uart interrupt in portserial.c
interrupt_isr_uart();

// main
void main(void) {

	// variable definition

	// program code
#if EN_LED
	gpio_pin_configure(LEDPIN,
		GPIO_PIN_CONFIG_OPTION_DIR_OUTPUT
		| GPIO_PIN_CONFIG_OPTION_OUTPUT_VAL_SET
		| GPIO_PIN_CONFIG_OPTION_PIN_MODE_OUTPUT_BUFFER_NORMAL_DRIVE_STRENGTH
		);
#endif

	interrupt_control_global_enable();
	initModbus(1, MB_BR_19200, MB_PARITY_NONE);

	while(1) {

#if EN_LED
		gpio_pin_val_complement(LEDPIN);
#endif

		(void) eMBPoll();

		/* Here we simply count the number of poll cycles. */
		usSRegInBuf[0]++;

	} // main loop
}
