/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
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
//	
//	The MCP23008-XP configuration:
//		dev-address = 01000:0000 // according to the factory's address
//		IOCON       = XX10: X11X // No sequential I/O, no interrupt generation
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
#include <avr/pgmspace.h>


//
// I2C I/O extender MCP23008XP's regirters
//
#define IODIR_ADDR 0x00
#define IOCON_ADDR 0x05
#define GPPU_ADDR  0x06
#define GPIO_ADDR  0x09


#define MC23008AP (MC23008_ADDR << 1) | 64


//
// MPC23008 configuration
//
#define IOCON_VALUE 38
#define IODIR_VALUE 1
#define GPU_VALUE   1


typedef enum _i2cMop {
	REGISTER_SELECTING,
	REGISTER_READING,
	REGISTER_UPDATING,
	I2C_NOP
} i2cMop;


//------------------------------------------------------------------------------------------------------------------------------
//                                                    M A I N
//------------------------------------------------------------------------------------------------------------------------------
int main() {
	i2cMop   mop = I2C_NOP;
	uint8_t  status = 1;
	uint8_t  regValue = 0;
	uint8_t  regAddr = 0;
	uint8_t  st = 0;
	uint8_t  gpioValue = 0;

	USART_Init(9600);
	_delay_ms(500);
	I2C_INIT
	_delay_ms(500);
	
	USART_writeString(PSTR("************* TEST START *************\n\r"), USART_FLASH);

	while (st < 127) {
		
		//logMsg(PSTR("[%d] ST=%d"), __LINE__, st);
		
		if (status == 0) {
			// ERROR!
			USART_writeString(PSTR("ERROR! I2C-BUS communication failed\n\n\r"), USART_FLASH);
			st = 127;
			mop = I2C_NOP;

		//
		// IOCON Register
		//

		} else if (st == 0) {
			I2C_STOP
			_delay_ms(1);
			regAddr = IOCON_ADDR;
			mop = REGISTER_SELECTING;
			st = 1;

		} else if (st == 1) {
			regValue = IOCON_VALUE;
			mop = REGISTER_UPDATING;
			st = 2;

		} else if (st == 2) {
			I2C_STOP
			_delay_ms(1);
			regAddr = IOCON_ADDR;
			mop = REGISTER_SELECTING;
			st = 3;

		} else if (st == 3) {
			regValue = 0;
			mop = REGISTER_READING;
			st = 4;

		} else if (st == 4) {
			mop = I2C_NOP;
			if ((regValue & IOCON_VALUE) == IOCON_VALUE) {
				USART_writeString(PSTR("[OK] Device configured (1/3)\n\r"), USART_FLASH);
				st = 10;
			} else {
				USART_writeString(PSTR("[ERROR] IOCON setting failed\n\r"), USART_FLASH);
				st = 127;
			}

		//
		// IODIR Register
		//

		} else if (st == 10) {
			I2C_STOP
			_delay_ms(1);
			regAddr = IODIR_ADDR;
			mop = REGISTER_SELECTING;
			st = 13;

		} else if (st == 13) {
			regValue = IODIR_VALUE;
			mop = REGISTER_UPDATING;
			st = 14;

		} else if (st == 14) {
			I2C_STOP
			_delay_ms(1);
			regAddr = IODIR_ADDR;
			mop = REGISTER_SELECTING;
			st = 15;

		} else if (st == 15) {
			mop = REGISTER_READING;
			st = 16;

		} else if (st == 16) {
			if (regValue == IODIR_VALUE) {
				USART_writeString(PSTR("[OK] Device configured (2/3)\n\r"), USART_FLASH);
				st = 20;
			} else {
				USART_writeString(PSTR("[ERROR] IODIR setting failed\n\r"), USART_FLASH);
				st = 127;
			}
			mop = I2C_NOP;


		//
		// GPPU Register
		//

		} else if (st == 20) {
			I2C_STOP
			_delay_ms(1);
			regAddr = GPPU_ADDR;
			mop = REGISTER_SELECTING;
			st = 21;

		} else if (st == 21) {
			regValue = GPU_VALUE;
			mop = REGISTER_UPDATING;
			st = 22;

		} else if (st == 22) {
			I2C_STOP
			_delay_ms(1);
			regAddr = GPPU_ADDR;
			mop = REGISTER_SELECTING;
			st = 23;

		} else if (st == 23) {
			mop = REGISTER_READING;
			st = 24;

		} else if (st == 24) {
			if (regValue == GPU_VALUE) {
				USART_writeString(PSTR("[OK] Device configured (3/3)\n\r"), USART_FLASH);
				st = 60;
			} else {
				USART_writeString(PSTR("[ERROR]GPU setting failed\n\r"), USART_FLASH);
				st = 127;
			}
			mop = I2C_NOP;


		//
		// GPIO Register
		//

		} else if (st == 60) {
			I2C_STOP
			_delay_ms(1);
			regAddr = GPIO_ADDR;
			mop = REGISTER_SELECTING;
			st = 61;

		} else if (st == 61) {
			regValue = 0;
			mop = REGISTER_READING;
			st = 62;

		} else if (st == 62) {
			I2C_STOP
			_delay_ms(1);
			regAddr = GPIO_ADDR;
			mop = REGISTER_SELECTING;
			st = 63;

		} else if (st == 63) {
			//update
			if ((regValue & 1) == 1) regValue |= (1 << 3);
			else                     regValue &= ~(1 << 3);

			mop = REGISTER_UPDATING;
			st = 64;

		} else if (st == 64) {
			I2C_STOP
			_delay_ms(1);
			regAddr = GPIO_ADDR;
			mop = REGISTER_SELECTING;
			st = 65;

		} else if (st == 65) {
			mop = REGISTER_READING;
			st = 66;
		
		} else if (st == 66) {
			if (gpioValue == regValue) {
				USART_writeString(PSTR("[OK] Inputs/outputs verified\n\r"), USART_FLASH);
				st = 60;
			} else {
				USART_writeString(PSTR("[ERROR] I/O acknowledge/setting failed \n\r"), USART_FLASH);
				st = 127;
			}
		}


		//------------------------------------------------------------------------------------------------------------------
		//                                            I 2 C   P R O C E D U R E S
		//------------------------------------------------------------------------------------------------------------------


		//
		// register selecting
		//
		if (mop == REGISTER_SELECTING) {
			status = 0;
			USART_writeString(PSTR("\nRegister selecting \n\r"), USART_FLASH);
			I2C_START(status)
			if (status) {
				I2C_WRITE(MC23008AP, status)
				if (status) 
					I2C_WRITE(regAddr, status)
			}
	

		//
		// register reading
		//
		} else if (mop == REGISTER_READING) {
			status = 0;
			regValue = 0;
			USART_writeString(PSTR("\nRegister reading\n\r"), USART_FLASH);
			I2C_START(status)
			if (status) {
				I2C_WRITE(MC23008AP|1, status)
				if (status) {
					I2C_READ(I2C_NACK, regValue, status)
					if (status) 
						logMsg(PSTR("%d register's value: %d"), regAddr, regValue);
				}
			}


		//
		// register updating
		//
		} else if (mop == REGISTER_UPDATING) {
			status = 0;
			USART_writeString(PSTR("\nRegister updating\n\r"), USART_FLASH);
			I2C_START(status)
			if (status == 1) {
				I2C_WRITE(MC23008AP, status);
				if (status == 1) {
					I2C_WRITE(regValue, status)
				}
			}

		}

		_delay_ms(100);
	}
			

	return(0);
}
