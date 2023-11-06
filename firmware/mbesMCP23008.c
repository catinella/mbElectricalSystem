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

static uint8_t MC2398_devAddr;

void init_MC2398 (uint8_t devAddr) {
	//
	// Description
	//	This function writes the device address inside the module. Every function belongs to this module will use the
	//	recorded device address (0-7)
	//
	//	[!] According to the datasheet, the most significant half byte is fixed (0100)
	//	   https://ww1.microchip.com/downloads/en/DeviceDoc/MCP23008-MCP23S08-Data-Sheet-20001919F.pdf
	//
	MC2398_devAddr = devAddr;
	MC2398_devAddr = MC2398_devAddr << 1;  // The first bit is used to set the I/O operation type (read/write)
	MC2398_devAddr |= 64;
	return;
}

void regSelecting_MC2398 (uint8_t regAddr) {
	//
	// Description
	//	This function allows you to select a MCD23008's register
	//
	I2C_Start();
	I2C_Write(MC2398_devAddr);  // LSB=0 --> writing operation
	I2C_Write(regAddr);

	return;
}


void regReading_MC2398 (uint8_t *value) {
	//
	// Description
	//	This function allows you to read the previousle selelected registers
	//
	I2C_Start();
	I2C_Write(MC2398_devAddr | 1);  // LSB=1 --> reading operation
	*value = I2C_Read(I2C_NACK);
	
	return;
}


void regSaving_MC2398 (uint8_t value) {
	//
	// Description
	//	This function allows you to write the previouse selelected registers
	//
	I2C_Start();
	I2C_Write(MC2398_devAddr);  // LSB=0 --> writing operation
	I2C_Write(value);

	return;
}
