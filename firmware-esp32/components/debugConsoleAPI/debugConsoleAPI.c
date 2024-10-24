/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: debugConsole.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This library is used to keep track of the I/O pins status.
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
#include <stdio.h>
#include <debugConsoleAPI.h>

#ifdef TARGET_AVR8
#include <avr/io.h>

#elifdef TARGET_ESP32
#include "driver/gpio.h"
#endif

static void _notify(const pinIdType pin, uint8_t value) {
	//
	// Description:
	//	This function sent the pin's information notification to the debug-console
	//
#ifdef TARGET_AVR8
	printf("%s:%d\n\r", pin, value); 
#elifdef TARGET_ESP32
	printf("GPIO_NUM_%d:%d\n\r", pin, value); 
#endif
	return;
}

//------------------------------------------------------------------------------------------------------------------------------
//                                         P U B L I C   F U N C T I O N S 
//------------------------------------------------------------------------------------------------------------------------------
uint8_t keepTrack_getGPIO (pinIdType pin) {
	//
	// Description:
	//	Pin's value reading
	//
	uint8_t value = 0;
	
#ifdef TARGET_AVR8
#error "ERROR! Not yet implemented"	

#elifdef TARGET_ESP32
	value = gpio_get_level(pin);
#endif

#ifdef DBGCON_KEEPTRACK
	_notify(pin, value);
#endif
	return(value);
}

void keepTrack_setGPIO (const pinIdType pin, uint8_t value) {
	//
	// Description:
	//	Pin's value setting
	//
#ifdef TARGET_AVR8
#error "ERROR! Not yet implemented"	

#elifdef TARGET_ESP32
	gpio_set_level(pin, value);
#endif

#ifdef DBGCON_KEEPTRACK
	_notify(pin, value);
#endif
	return;
}
