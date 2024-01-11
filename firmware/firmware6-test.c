
/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   firmware6-test.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This test has been developped to ckeck for pins management functions belong to the mbesUtilities library
//
//	This test will read two input-pin's value and will copy the value in two output-pin. The following table show you the
//	pins used by the test and their association
//
//	+--------------+-------------+--------------+
//	|   Input pin  |    Device   |  Output pin  |
//	+--------------+-------------+--------------+
//	| GP0 (pin-10) | MCP23008-XP | GP3 (pin-13) |
//	| C3  (pin 25) | AT-Mega-16  | C2  (pin 24) |
//	+--------------+-------------+--------------+
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
#include <mbesMCP23008.h>

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#define IN_A  "C3"
#define IN_B  "00"
#define OUT_A "C2"
#define OUT_B "03"

typedef enum _fsm {
	INITIALIZATION,
	PA_READING,	
	PB_READING,	
	PA_WRITING,
	PB_WRITING,
	I2CBUS_ERROR,
	END
} fsm;


#if DEBUG > 0
#define LOGMSG(X)      USART_writeString(PSTR(X), USART_FLASH);
#else
#define LOGMSG(X)      ;
#endif


//------------------------------------------------------------------------------------------------------------------------------
//                                                         M A I N
//------------------------------------------------------------------------------------------------------------------------------

int main() {
	fsm     state = INITIALIZATION;
	uint8_t pinStatus = 0;
	uint8_t errCounter = 0;

	// Serial console initialization
	USART_Init(9600);

	// JTAG disabling
	MCUCR |= (1 << JTD);
	MCUCR |= (1 << JTD);

	// PINs initialization
	pinDirectionRegister(IN_A, INPUT);
	pinDirectionRegister(OUT_A, OUTPUT);

	while (state != END) {
		
		if (state == INITIALIZATION) {
			LOGMSG("\nInitialization process starting...\n\r");
			
			if (init_MCP23008(MCP23008_ADDR) == 0) {
				// ERROR
				USART_writeString(PSTR("ERROR! I2C-BUS initialization failed\n\n\r"), USART_FLASH);
				state = I2CBUS_ERROR;
				
			} else if (pinDirectionRegister(IN_B, INPUT)  == 0 || pinDirectionRegister(OUT_B, OUTPUT) == 0) {
				// ERROR
				USART_writeString(PSTR("ERROR! MCP23008 configuration failed\n\n\r"), USART_FLASH);
				state = I2CBUS_ERROR;
				
			} else
				state = PA_READING;


		} else if (state == PA_READING) {
			LOGMSG("\nMCU's PINs reading...\n\r");
			// [!] Because IN-A and OUT-A are MCU's pins, you don't need to evaluate the error
			getPinValue(IN_A, &pinStatus);
			state = PA_WRITING;


		} else if (state == PA_WRITING) {
			LOGMSG("\nMCU's PINs writing...\n\r");
			// [!] No error evaluation
			setPinValue(OUT_A, pinStatus);
			state = PB_READING;

			
		} else if (state == PB_READING) {
			LOGMSG("\nMCP23008's PINs reading...\n\r");
			if (getPinValue(IN_B, &pinStatus) == 0) {
				// ERROR
				USART_writeString(PSTR("ERROR! MCP23008's registers reading failed\n\n\r"), USART_FLASH);
				state = I2CBUS_ERROR;
			} else
				state = PB_WRITING;


		} else if (state == PB_WRITING) {
			LOGMSG("\nMCP23008's PINs writing...\n\r");
			if (setPinValue(OUT_B, pinStatus) == 0) {
				// ERROR
				USART_writeString(PSTR("ERROR! MCP23008's registers updating failed\n\n\r"), USART_FLASH);
				state = I2CBUS_ERROR;
			} else {
				state = PA_READING;
				errCounter = 0;
			}
			
			
		} else if (state == I2CBUS_ERROR) {
			LOGMSG("\nErrors number evaluation\n\r");
			if (errCounter > 10)
				state = END;
			else {
				errCounter++;
				state = INITIALIZATION;
			}
		}

		#if DEBUG > 0
		_delay_ms(100);
		#else
		_delay_ms(1);
		#endif
	}

	return(0);
}
