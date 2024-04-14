
/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   firmware7a-test.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This file is not exactly a test, it is mora an utility. In fact, using this test you can check for every input. 
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
#include <mbesSelector.h>
#include <mbesPinsMap.h>

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>


#define WITH_SELECTOR 1

#if DEBUG > 0
#define LOGMSG(X,Y)   logMsg(PSTR(X), Y);
#else
#define LOGMSG(X, Y)  ;
#endif

#define PIN_INPUT   i_ENGINEON
#define PIN_OUTPUT  o_KEEPALIVE

int main() {

	// Pins declaration
	#if WITH_SELECTOR > 0
	struct mbesSelector mySelector;
	#endif
	uint8_t pinValue = 1;

	
	// Serial console initialization
	USART_Init(9600);


	// JTAG disabling
	MCUCR |= (1 << JTD);
	MCUCR |= (1 << JTD);


	// MCU's PINs initialization
	#if WITH_SELECTOR > 0
	mbesSelector_init(&mySelector, SWITCH, PIN_INPUT);
	#else
	pinDirectionRegister(PIN_INPUT,   INPUT);
	#endif
	pinDirectionRegister(PIN_OUTPUT, OUTPUT);

	
	while (1) {
		// MCU's pin reading...
		#if WITH_SELECTOR > 0
		pinValue = mbesSelector_get(mySelector) ? 1 : 0;
		#else
		getPinValue(PIN_INPUT, &pinValue);
		#endif
		LOGMSG("VALUE: %d", pinValue);

		// Led light turning ON/OFF
		#if WITH_SELECTOR > 0
		setPinValue(PIN_OUTPUT, mbesSelector_get(mySelector));
		#else
		setPinValue(PIN_OUTPUT, pinValue);
		#endif
		
		_delay_ms(100);
		#if WITH_SELECTOR > 0
		mbesSelector_update(&mySelector);
		#endif
	}
	
	return(0);
}
