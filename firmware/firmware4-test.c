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
#include <mbesUtilities.h>
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


#define MC23008AP (MC23008_ADDR << 1) | 64

int main() {
	uint8_t regValue = 0;
	uint8_t fsm = 0;
	uint8_t status;

	USART_Init(2400);
	I2C_init();
	_delay_ms(100);
	
	USART_writeString("************* TEST START *************\n\r");

	while(fsm < 127) {
		
		//
		// Initialization: "IODIR" register selecting
		//
		if (fsm == 0) {
			USART_writeString("IODIR register selecting \n\r");
			I2C_START(status)
			if (status == 1 && I2C_Write(MC23008AP) && I2C_Write(IODIR_ADDR)) 
				fsm = 1;

			else {
				// ERROR!
				USART_writeString("ERROR!I cannot select IODIR register\n\n\r");
				fsm = 127;
			}
	

		//
		// Initialization: "IODIR" register reading
		//
		} else if (fsm == 1) {
			USART_writeString("IODIR register reading\n\r");
			I2C_START(status)
			if (status == 1 && I2C_Write(MC23008AP|1) && I2C_Read(I2C_NACK, &regValue))
				fsm = 2;
			else {
				// ERROR!
				USART_writeString("ERROR!I cannot read IODIR register\n\n\r");
				fsm = 127;
			}


		//
		//
		} else if (fsm == 2) {
			logMsg ("IODIR: %d", regValue);

			// "IODIR" register changing
			regValue &= ~(1 << 3);               // GP3 (bit0 --> OUTPUT)
			regValue |=  (1 << 0);               // GP0 (bit1 --> INPUT)
		
			// "IODIR" register saving
			USART_writeString("IODIR register updaing\n\r");
			I2C_START(status)
			if (status == 1 && I2C_Write(MC23008AP) && I2C_Write(regValue))
				fsm = 3;
			else {
				// ERROR!
				USART_writeString("ERROR!I cannot update the  IODIR register\n\n\r");
				fsm = 127;
			}

		
		//
		// STOP
		//
		} else if (fsm == 3) {
			I2C_STOP


		//
		// GPIO register selecting
		//
		} else if (fsm == 20 || fsm == 23) {
			USART_writeString("GPIO register selecting\n\r");
			I2C_START(status)
			if (status == 1 && I2C_Write(MC23008AP) && I2C_Write(GPIO_ADDR)) {
				USART_writeString("OK\n\n\r");
				if      (fsm == 20) fsm = 21;
				else if (fsm == 23) fsm = 24;
			} else {
				// ERROR!
				USART_writeString("ERROR!\n\n\r");
				fsm = 29;
			}

		//
		// GPIO register reading
		//
		} else if (fsm == 21) {
			USART_writeString("GPIO data reading\n\r");
			I2C_START(status)
			if (status == 1 && I2C_Write((MC23008AP|1)) && I2C_Read(I2C_NACK, &regValue)){ 
				logMsg ("GPIO: %d", regValue);
				fsm = 22;
			} else {
				// ERROR!
				USART_writeString("ERROR!I cannot read GPIO\n\n\r");
				fsm = 29;
			}


		//
		// fsm == 22 --> STOP
		//


		//
		// fsm == 23 --> Reg selecting
		//


		//
		// GPIO updating
		//
		} else if (fsm == 4) {
			if ((regValue & 1) == 0)          // GP0 == 0 ?
				regValue &= ~(1 << 3);      // GP3 = 0
			else
				regValue |= (1 << 3);       // GP3 = 1
		

			// GPIO register saving
			USART_writeString("GPIO register updating\n\r");
			I2C_START(status)
			if (status == 1 && I2C_Write(MC23008AP) && I2C_Write(regValue)) {
				USART_writeString("OK\n\n\r");
				fsm = 25;
			} else {
				// ERROR!
				USART_writeString("ERROR!I cannot update GPIO\n\n\r");
				fsm = 29;
			}


		//
		// Stop 
		//
		} else if (fsm == 25 || fsm == 2) {
			I2C_STOP
		

		//
		// Error (BUS reset)
		//
		} else if (fsm == 29) {
			I2C_BUSRESET
		}

		_delay_ms(10);
	}
			
	return(0);
}
