/*
 * modbus_slave.h
 *
 *  Created on: 03/05/2017
 *      Author: andru
 */

#ifndef MODBUS_SLAVE_H_
#define MODBUS_SLAVE_H_

/* -----------------------Slave Defines -------------------------------------*/
#define MB_MODE						MB_RTU			// rtu mode only
#define MB_PORT						1				// 1 port only

#define S_DISCRETE_INPUT_START      0
#define S_DISCRETE_INPUT_NDISCRETES 6
#define S_COIL_START                0
#define S_COIL_NCOILS               6
#define S_REG_INPUT_START           0
#define S_REG_INPUT_NREGS           6
#define S_REG_HOLDING_START         0
#define S_REG_HOLDING_NREGS         6

#include "FreeModbus/port/port.h"

//Slave mode:DiscreteInputs variables
extern USHORT   usSDiscInStart;
extern UCHAR    ucSDiscInBuf[];
//Slave mode:Coils variables
extern USHORT   usSCoilStart;
extern UCHAR    ucSCoilBuf[];
//Slave mode:InputRegister variables
extern USHORT   usSRegInStart;
extern SHORT   usSRegInBuf[];
//Slave mode:HoldingRegister variables
extern USHORT   usSRegHoldStart;
extern SHORT   usSRegHoldBuf[];

typedef enum {
	MB_BR_2400,
	MB_BR_4800,
	MB_BR_9600,
	MB_BR_19200,
	MB_BR_38400,
} mb_bitrate_t;

typedef enum {
    MB_PARITY_NONE, /*!< No parity. */
    MB_PARITY_ODD,  /*!< Odd parity. */
    MB_PARITY_EVEN  /*!< Even parity. */
} mb_parity_t;

BOOL initModbus (UCHAR addr, mb_bitrate_t bitrate, mb_parity_t parity);

#endif /* MODBUS_SLAVE_H_ */
