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
#ifndef MBESI2C
#define MBESI2C

#include <stdint.h>
#include <util/twi.h>
#include <mbesUtilities.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG == 0
#define LOGTRACE
#define TIMEOUTMSG
#else
#define LOGTRACE   logMsg("%s(): start", __FUNCTION__);
#endif

#define TIMEOUTMSG logMsg("%s(): timeout", __FUNCTION__);
#define I2C_CLOCK_FREQ 10000
#define I2C_TIMEOUT    100
#define I2C_INTPULLUP  0

typedef enum _mbesI2CopType {
	I2C_ACK,
	I2C_NACK
} mbesI2CopType;


//
// Functions implemented as macros
//
#define I2C_STOP {                                    \
	LOGTRACE                                        \
	TWCR =(1 << TWINT) | (1 << TWEN)| (1 << TWSTO); \
}

#define I2C_BUSRESET {           \
	LOGTRACE  _delay_ms(200);  \
	for (int t=3; t>0; t--) {  \
		_delay_ms(100);      \
		I2C_STOP;            \
	}                          \
}

#define I2C_START(var) {                                \
	LOGTRACE                                          \
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA); \
	var = waitForTwint();                             \
}




//
// Functions prototypes
//

void    I2C_init     ();
uint8_t I2C_Write    (uint8_t data);
uint8_t I2C_Read     (mbesI2CopType optType, uint8_t *data);

uint8_t waitForTwint ();

#endif
