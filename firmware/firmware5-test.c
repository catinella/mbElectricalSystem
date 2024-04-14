/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   firmware5-test.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This test has been developped to ckeck for the mbesMCP23008 library. To check for lower level library, consider the
//	firmware4-test.c firmware
//
//	This test will read the MCP23008-XP's GP0 (pin-10) status and will set the GP3 (pin-13) at the same status.

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
#include <mbesMCP23008.h>

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#define IODIR_VALUE 1
#define GPPU_VALUE  1

typedef enum _fsm {
	INITIALIZATION,
	CHECK_IODIR,
	CHECK_GPPU,
	GPIO_READ,
	GPIO_WRITE,
	BUSERROR,
	END
} fsm;

//------------------------------------------------------------------------------------------------------------------------------
//                                                         M A I N
//------------------------------------------------------------------------------------------------------------------------------

int main() {
	fsm     state = INITIALIZATION;
	uint8_t regValue = 0;
	uint8_t errCounter = 0;

	// Serial console initialization
	USART_Init(9600);


	while (state != END) {

		if (state == INITIALIZATION) {
			if (
				init_MCP23008(MCP23008_ADDR) == 0           ||
				regSelecting_MCP23008(MCP23008_IODIR) == 0  ||
				regSaving_MCP23008(IODIR_VALUE) == 0        ||
				regSelecting_MCP23008(MCP23008_GPPU) == 0   ||
				regSaving_MCP23008(GPPU_VALUE) == 0
			) {
				// ERROR
				USART_writeString(PSTR("ERROR! I2C-BUS initialization failed\n\n\r"), USART_FLASH);
				state = BUSERROR;
			} else
				state = CHECK_IODIR;


		} else if (state == CHECK_IODIR) {
			if (
				regSelecting_MCP23008(MCP23008_IODIR) == 0  ||
				regReading_MCP23008(&regValue) == 0         ||
				regValue != IODIR_VALUE
			) {
				// ERROR
				USART_writeString(PSTR("ERROR! IODIR register setting failed\n\n\r"), USART_FLASH);
				state = BUSERROR;
			} else
				state = CHECK_GPPU;

		} else if (state == CHECK_GPPU) {
			if (
				regSelecting_MCP23008(MCP23008_GPPU) == 0  ||
				regReading_MCP23008(&regValue) == 0        ||
				regValue != GPPU_VALUE
			) {
				// ERROR
				USART_writeString(PSTR("ERROR! GPPU register setting failed\n\n\r"), USART_FLASH);
				state = BUSERROR;
			} else
				state = GPIO_READ;


		} else if (state == GPIO_READ) {
			if (
				regSelecting_MCP23008(MCP23008_GPIO) == 0  ||
				regReading_MCP23008(&regValue) == 0
			) {
				// ERROR
				USART_writeString(PSTR("ERROR! GPIO register reading failed\n\n\r"), USART_FLASH);
				state = BUSERROR;

			} else
				state = GPIO_WRITE;

			
		} else if (state == GPIO_WRITE) {
			if ((regValue & 1) == 1)
				regValue |= (1 << 3);
			else {
				regValue &= ~(1 << 3);
				#if DEBUG > 0
				USART_writeString(PSTR("Pushed button!!\n\n\r"), USART_FLASH);
				#endif
			}
			
			if (regSaving_MCP23008(regValue) == 0) {
				// ERROR
				USART_writeString(PSTR("ERROR! GPIO register reading failed\n\n\r"), USART_FLASH);
				state = BUSERROR;
			} else {
				errCounter = 0;
				state = GPIO_READ;
			}


		} else if (state == BUSERROR) {
			if (errCounter > 64) {
				USART_writeString(PSTR("[!] Too many errors!!!! Device disabling\n\n\r"), USART_FLASH);
				state = END;
			} else 
				errCounter++;
		}

		#if DEBUG > 0
		_delay_ms(100);
		#else
		_delay_ms(1);
		#endif

	} // "while" loop

	return(0);
}
