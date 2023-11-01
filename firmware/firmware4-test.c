/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   firmware4-test.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This test has been developped to ckeck for the I/O extension chip MCP23008-XP. This device is connected to the ATmega16
//	using I2C BUS.
//
//	This test will read the MCP23008-XP's GP0 (pin-10) status and will set the GP3 (pin-13) at the same status.
//	The MCP23008-XP's address is 0000:0000
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

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

//
// I2C I/O extender MCP23008XP's regirters
//
#define IODIR_ADDR 0x00
#define GPIO_ADDR  0x09
#define GPPU_ADDR  0x06


int main() {
	uint8_t regValue = 0;
 
	I2C_init();
	
	// "IODIR" register selecting
	I2C_Start();
	I2C_Write(0);                 // LSB=0 --> writing operation
	I2C_Write(GPIO_ADDR);
	
	// "IODIR" register reading
	I2C_Start();
	I2C_Write(1);                 // LSB=1 --> reading operation
	regValue = I2C_Read(I2C_NACK);
	
	// "IODIR" register changing
	regValue &= ~(1 << 3);          // GP3 (bit0 --> OUTPUT)
	regValue |=  (1 << 0);          // GP0 (bit1 --> INPUT)
		
	// "IODIR" register saving
	I2C_Start();
	I2C_Write(0);               // LSB=0 --> writing operation
	I2C_Write(regValue);

	I2C_Stop();

		
	while(1) {
		
		// "GPIO" register selecting
		I2C_Start();
		I2C_Write(0);                   // LSB=0 --> writing operation
		I2C_Write(GPIO_ADDR);
		
		// "GPIO" register reading
		I2C_Start();
		I2C_Write(1);                   // LSB=1 --> reading operation
		regValue = I2C_Read(I2C_NACK);
		
		// "GPIO" changing
		if ((regValue & 1) == 0)        // GP0 == 0 ?
			regValue &= ~(1 << 3);    // GP3 = 0
		else
			regValue |= (1 << 3);     // GP3 = 1

		// "IODIR" register saving
		I2C_Start();
		I2C_Write(0);                    // LSB=0 --> writing operation
		I2C_Write(regValue);

		I2C_Stop();

		_delay_ms(500);
	}
			
	return(0);
}
