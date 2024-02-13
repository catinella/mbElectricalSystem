/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesI2C.h
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This file contains fuctions to manage external devices connected with I2C bus. In order to reduce the used memory
//	the functions has been implemented as macros. It reduces the memory stack using.
//
//	       +-------+-------+-------+-------+-------+-------+------+------+
//	TWCR : | TWINT | TWEA  | TWSTA | TWSTO | TWWC  | TWEN  |      | TWIE |
//	       +-------+-------+-------+-------+-------+-------+------+------+
//	       TWINT: TWI Interrupt Flag
//	       TWEA:  TWI Enable Acknowledge Bit
//	       TWSTA: TWI START Condition Bit
//	       TWSTO: TWI STOP Condition Bit
//	       TWWC:  TWI Write Collision Flag
//	       TWEN:  TWI Enable Bit
//	       [////]
//	       TWIE: TWI Interrupt Enable
//	
//	
//	DEBUG mode
//	==========
//	In order to enable the debug messages, you have to set the DEBUG symbol to a value that is greather then 1 
//	[!] Consider the verbouse messages on serial console can have impact on timing and performance data.
//	
//	
//	Error codes convention
//	======================
//	All functions that returns a uint8_t error codes, respect the following convention:
//		0 // Warning or Error
//		1 // Success
//	
//
//
// License:
//	Copyright (C) 2023 Silvano Catinella <catinella@yahoo.com>
//
//	This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
//	License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
//	version.
//
//	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
//	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with this program. If not, see
//		<https://www.gnu.org/licenses/gpl-3.0.txt>.
//
------------------------------------------------------------------------------------------------------------------------------*/
#ifndef MBESI2C
#define MBESI2C

#include <stdint.h>
#include <util/twi.h>
#include <mbesUtilities.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#ifndef MBES_I2C_DEBUG
#define MBES_I2C_DEBUG 0
#endif

#if MBES_I2C_DEBUG
#define LOGTRACE(X)        logMsg(PSTR("%s()"), X);
#define LOGTRACEWP(X, Y)   logMsg(PSTR("%s(%d)"), X, Y);
#else
#define LOGTRACE(X)         ;
#define LOGTRACEWP(X, Y)    ;
#endif

#define TIMEOUTMSG         logMsg(PSTR("[!] Timeout"));
#define I2C_CLOCK_FREQ     10000
#define I2C_TIMEOUT        100
#define DELAYSTEP          2


typedef enum _mbesI2CopType {
	I2C_ACK,
	I2C_NACK
} mbesI2CopType;


//
// Functions implemented as macros
//

//------------------------------------------------------------------------------------------------------------------------------
//                                                   M A C R O S
//------------------------------------------------------------------------------------------------------------------------------

//
// Description:
//	I2C Initialization
//	TWCR = 0x00;  // Interrupts disabling
//	TWSR = 0x00;  // Prescaler = 1
//
#define I2C_INIT {                                             \
	LOGTRACE("I2C_INIT")                                     \
	TWCR = 0x00;                                             \
	TWBR = (uint8_t)(((F_CPU / I2C_CLOCK_FREQ) - 16) / 2);   \
	TWSR = 0x00;                                             \
	TWCR = 1 << TWEN;                                        \
}


//
// Description:
//	It enables the internal pull-up resistors for the I2C BUS lines.
//		SCL: PC0(PIN=22)
//		SDA: PC1(PIN=23)
//
#define I2C_INTPULLUP {          \
	LOGTRACE("I2C_INITPULLUP") \
	PORTC |= 1;                \
	PORTC |= 1 << 1;           \
}



//
// Description:
//	It sends a stop signal to the I2C BUS connected device.
//	[!] No ACK is required by the device
//
#define I2C_STOP {                                    \
	LOGTRACE("I2C_STOP")                            \
	TWCR =(1 << TWINT) | (1 << TWEN)| (1 << TWSTO); \
	while(TWCR & (1 << TWSTO));                     \
}


//
// Description:
//	This procedure resets the I2C BUS
//
#define I2C_BUSRESET {           \
	LOGTRACE("I2C_BUSRESET")   \
	_delay_ms(200);            \
	for (int t=3; t>0; t--) {  \
		_delay_ms(100);      \
		I2C_STOP;            \
	}                          \
}


//
// Description:
//	It sends a start signal to the I2C BUS connected device and waits for the ACK signal.
//
#define I2C_START(status) {                             \
	LOGTRACE("I2C_START")                             \
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA); \
	WAIT4TWINT(status)                                \
	status = status > 0 ? 1 : 0;                      \
}


//
// Description:
//	It sends the argument defined byte using the I2C BUS, and returns the transmission status
//
//	When an event requiring the attention of the application occurs on the TWI bus, the TWI Interrupt Flag (TWINT) is
//	asserted. In the next clock cycle, the TWI Status Register (TWSR) is updated with a status code identifying the
//	event. The TWSR only contains relevant status information when the TWI Interrupt Flag is asserted.
//
#define I2C_WRITE(data_var, status_var) { \
	LOGTRACEWP("I2C_WRITE", data_var)     \
	TWDR = data_var;                      \
	TWCR = (1 << TWINT) | (1 << TWEN);    \
	WAIT4TWINT(status)                    \
	status = status > 0 ? 1 : 0;          \
}


//
// Description:
//	It is used to wait for data received
//
//	------+             +---------
//	<user>|    <MCU>    | <MCU>
//	      +-------//----+
//	  (op. start)    (op. end)
//
//
#define WAIT4TWINT(tout) {                             \
	tout = I2C_TIMEOUT / DELAYSTEP;                  \
	while ((TWCR & (1 << TWINT)) == 0 && tout > 0) { \
		_delay_ms(DELAYSTEP);                      \
		tout--;                                    \
	}                                                \
	if (tout == 0) TIMEOUTMSG                        \
	_delay_ms(DELAYSTEP);                            \
}


//
// Description:
//	It reads a byte from the I2C bus and writes it in the macro's argument defined variable.
//	The fist argument defines the rading type /with or without ACK), the second is where the register's content will be witten,
//	the third reports the procedure status (failed=0, success=1)
//
#define I2C_READ(optType, data, status) {                     \
	LOGTRACE("I2C_READ")                                    \
	if (optType == I2C_ACK) {                               \
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);  \
		LOGTRACE("ACK will be expected");                 \
	} else {                                                \
		TWCR = (1 << TWINT) | (1 << TWEN);                \
		LOGTRACE("NO ACK will be expected");              \
	}                                                       \
	WAIT4TWINT(status)                                      \
	status = status > 0 ? 1: 0;                             \
	data = TWDR;                                            \
}



#endif
