/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesI2C.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This file contains functions to manage external devices connected with I2C bus
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
//	Error codes convention
//	======================
//	All functions that returns a uint8_t error codes, respect the following convention:
//		0 // Warning or Error
//		1 // Success
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
#include <mbesHwConfig.h>

// AVR Libraries
#include <avr/io.h>
#include <util/delay.h>

#include <mbesI2C.h>

#define DELAYSTEP 10

#define TOUTFLAGSRESET TWCR = (1 << TWINT) | (1 << TWEN)

//------------------------------------------------------------------------------------------------------------------------------
//                                       P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

uint8_t waitForTwint() {
	//
	// Description:
	//	It is used to wait for data received
	//
	//	------+             +---------
	//	<user>|    <MCU>    | <MCU>
	//	      +-------//----+
	//	  (op. start)    (op. end)
	//
	// Returned value
	//	0  WARNING! timeout achieved
	//	1  OK, data is ready
	//
	uint16_t tout = I2C_TIMEOUT / DELAYSTEP;
	
	while ((TWCR & (1 << TWINT)) == 0 && tout > 0) {
		_delay_ms(DELAYSTEP);
		tout--;
	}
	if (tout == 0) TIMEOUTMSG(__FUNCTION__)
	return(tout > 0 ? 1 : 0);
}


void I2C_init (void) {
	//
	// Description:
	//	I2C Initialization
	//
	LOGTRACE(__FUNCTION__)
	TWCR = 0x00;                            // Interrupts disabling
	TWBR = (uint8_t)(((F_CPU / I2C_CLOCK_FREQ) - 16) / 2);
	TWSR = 0x00;                            // Prescaler = 1
	TWCR = 1 << TWEN;

#if I2C_INTPULLUP == 1
	// Internal pull-up resistors
	PORTC |= 1;      // SCL: PC0(PIN=22)
	PORTC |= 1 << 1; // SDA: PC1(PIN=23)
#endif

	return;
}


uint8_t I2C_Write (uint8_t data) {
	//
	// Description:
	//	It sends the argument defined byte using the I2C BUS, and returns the transmission status
	//
	//	When an event requiring the attention of the application occurs on the TWI bus, the TWI Interrupt Flag (TWINT) is
	//	asserted. In the next clock cycle, the TWI Status Register (TWSR) is updated with a status code identifying the
	//	event. The TWSR only contains relevant status information when the TWI Interrupt Flag is asserted.
	//
	LOGTRACE(__FUNCTION__)
	TWDR = data;
	TOUTFLAGSRESET ;
	return(waitForTwint());
}


uint8_t I2C_Read (mbesI2CopType optType, uint8_t *data) {
	//
	// Description:
	//	It reads a byte from the I2C bus and returns it. 
	//
	uint8_t err = 0;
	LOGTRACE(__FUNCTION__)

	if (optType == I2C_ACK) {
		TOUTFLAGSRESET | (1 << TWEA); 
		logMsg("ACK will be expected");
	} else {
		TOUTFLAGSRESET ;
		logMsg("NO ACK will be expected");
	}

	// Waiting  for data cknowledge
	err = waitForTwint();
	
	*data = TWDR;

	return(err);
}


