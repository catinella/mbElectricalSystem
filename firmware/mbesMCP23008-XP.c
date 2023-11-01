/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesMCP23008-XP.c
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
#include <mbesMCP23008-XP.h>

void regSelecting (uint8_t devAddr, uint8_t regAddr, mcp23008_regMode mode) {
	//
	// Description
	//	This function allows you to open a MCD23008's register in read or write mode
	//
	I2C_Start();
	if (mode == mcp23008_regReading) {
		I2C_Write(devAddr << 1);         // LSB=1 --> reading operation
		devAddr |= 1;
	} else {
		I2C_Write(devAddr << 1);         // LSB=0 --> writing operation
	}
	I2C_Write(regAddr);

	return;
}


void regReading (uint8_t devAddr, uint8_t *value) {
	//
	// Description
	//	This function allows you to read the previousle selelected registers
	//
	I2C_Start();
	I2C_Write((devAddr << 1) | 1);          // LSB=1 --> reading operation
	*value = I2C_Read(I2C_NACK);
	
	return;
}


void regSaving (uint8_t devAddr, uint8_t value) {
	I2C_Start();
	I2C_Write(devAddr << 1);               // LSB=0 --> writing operation
	I2C_Write(value);

	return;
}
