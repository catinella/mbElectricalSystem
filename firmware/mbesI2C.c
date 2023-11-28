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


//------------------------------------------------------------------------------------------------------------------------------
//                                       P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

//
// TODO: using the macros to get the typical function APIs (when the RAM size is not so important or when the flash size is the
//       important one)
