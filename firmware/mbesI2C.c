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

// AVR Libraries
#include <avr/io.h>
#include <util/twi.h>

#include <mbesHwConfig.h>
#include <mbesUtilities.h>
#include <mbesI2C.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG == 0
#define LOGTRACE
#else
#define LOGTRACE   logMsg("%s()", __FUNCTION__);
#endif

void I2C_init (void) {
	//
	// Description:
	//	I2C Initialization
	//
	LOGTRACE
	TWCR = 0x00;                            // Interrupts disabling
	TWBR = (uint8_t)(((F_CPU / I2C_CLOCK_FREQ) - 16) / 2);
	TWSR = 0x00;                            // Prescaler = 1
	TWCR |= (1 << TWEN);                    // I2C module enabling...
	
	return;
}


uint8_t I2C_Write (uint8_t data) {
	//
	// Description:
	//	It sends the argument defined byte using the I2C BUS, and returns the transmission status
	//
	LOGTRACE
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	// Waiting for the operation end
	while (!(TWCR & (1 << TWINT)));

	return(TWSR & 0xF8);
}


uint8_t I2C_Read (mbesI2CopType optType) {
	//
	// Description:
	//	It reads a byte from the I2C bus and returns it. 
	//
	LOGTRACE

	if (optType == I2C_ACK) {
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); 
		logMsg("ACK will be expected");
	} else {
		TWCR = (1 << TWINT) | (1 << TWEN);
		logMsg("NO ACK will be expected");
	}

	// Waiting  for data cknowledge
	while (!(TWCR & (1 << TWINT)));

	return(TWDR);
}


void I2C_Stop (void) {
	//
	// Description:
	//	It seands a STOP marker to the remote device on I2C
	//
	LOGTRACE

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	return;

}


void I2C_Start (void) {
	//
	// Description:
	//	It seands a START marker to the remote device on I2C
	//
	LOGTRACE

	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return;
}

