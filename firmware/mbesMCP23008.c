/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesMCP23008.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This lib allows to use the MCP23008-XP I/O bus extension in easy way
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
#include <mbesI2C.h>
#include <mbesMCP23008.h>

#ifndef MBES_MCP23008_DEBUG
#define MBES_MCP23008_DEBUG 0
#endif

#if MBES_MCP23008_DEBUG > 1
#define LOGMSG(X)       logMsg(PSTR(X));
#define W1V_LOGMSG(X,Y) logMsg(PSTR(X), Y);
#else
#define LOGMSG(X)      ;
#define W1V_LOGMSG(X,Y) ;
#endif

#if MBES_MCP23008_DEBUG > 0
#define LOGERR   logMsg(PSTR("ERROR! in %s(%d)"), __FUNCTION__, __LINE__);
#else
#define LOGERR   ;
#endif

static uint8_t MCP23008_devAddr;

//------------------------------------------------------------------------------------------------------------------------------
//                                         P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

uint8_t init_MCP23008 (uint8_t devAddr) {
	//
	// Description
	//	This function writes the device address inside the module. Every function belongs to this module will use the
	//	recorded device address (0-7)
	//
	//	[!] According to the datasheet, the most significant half byte is fixed (0100)
	//	   https://ww1.microchip.com/downloads/en/DeviceDoc/MCP23008-MCP23S08-Data-Sheet-20001919F.pdf
	//
	uint8_t ec = 0;
	uint8_t reg = 0;

	W1V_LOGMSG("0x0%d-device initialization", devAddr);
	
	//
	// Device address setting
	//
	MCP23008_devAddr = devAddr;
	MCP23008_devAddr = MCP23008_devAddr << 1;  // The first bit is used to set the I/O operation type (read/write)
	MCP23008_devAddr |= 64;


	// I2C bus initialization
	I2C_INIT

	
	//
	// Sequential access disabling
	//
	if (
		regSelecting_MCP23008(MCP23008_IOCON)    &&
		regSaving_MCP23008(MCP23008_IOCON_VALUE) &&
		regSelecting_MCP23008(MCP23008_IOCON)    &&
		regReading_MCP23008(&reg)                &&
		reg == MCP23008_IOCON_VALUE
	) {
		// IOCON has been successfully configured
		LOGMSG("IOCON register verified")

		if (
			regSelecting_MCP23008(MCP23008_IODIR) &&
			regSaving_MCP23008(0)                 &&
			regReading_MCP23008(&reg)             &&
			reg == 0
		) {
			// IODIR has been successfully configured
			LOGMSG("IODIR register verified")

			if (
				regSelecting_MCP23008(MCP23008_GPPU) &&
				regSaving_MCP23008(0)                &&
				regReading_MCP23008(&reg)            &&
				reg == 0
			) {
				// IODIR has been successfully configured
				LOGMSG("GPPU register verified")
				ec = 1;
			} else
				LOGERR
		} else
			LOGERR
	} else
		LOGERR
		
	return(ec);
}

uint8_t regSelecting_MCP23008 (uint8_t regAddr) {
	//
	// Description
	//	This function allows you to select a MCD23008's register
	//
	uint8_t status = 0;
	I2C_STOP
	_delay_ms(1);
	W1V_LOGMSG("0x0%d-register selecting...", regAddr);
	I2C_START(status)
	if (status) I2C_WRITE(MCP23008_devAddr, status)
	if (status) I2C_WRITE(regAddr, status)

	return(status); 
}


uint8_t regReading_MCP23008 (uint8_t *value) {
	//
	// Description
	//	This function allows you to read the previousle selelected registers
	//
	uint8_t status = 0;
	LOGMSG("Register reading...");
	I2C_START(status)
	if (status) {
		I2C_WRITE(MCP23008_devAddr|1, status)
		if (status) {
			I2C_READ(I2C_NACK, *value, status)
			if (status) W1V_LOGMSG("Register's value: %d", *value)
		}
	}
	return(status);
}


uint8_t regSaving_MCP23008 (uint8_t value) {
	//
	// Description
	//	This function allows you to write the previouse selelected registers
	//
	uint8_t status = 0;
	
	LOGMSG("Register updating...");
	I2C_START(status)
	if (status == 1) {
		I2C_WRITE(MCP23008_devAddr, status);
		if (status == 1)
			I2C_WRITE(value, status)
			if (status) W1V_LOGMSG("New register's value: %d", value)
	}
	
	return(status);
}
