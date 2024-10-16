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
//	When the degìbug session become a too much chaotic one, it is useful to monitor the PINs status. This small lib allows
//	you to achieve the result. The function keepTrack() must be called in all low-level functions where they set or get
//	a value to/from a pin. The function will send an ascii log with the following llayout ""<PIN-name>:<value>", to an
//	external (Perl) process, that will show all pins status.
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


void keepTrack_strID(const char *code, uint8_t value) {
	//
	// Description:
	//	This function is a platform independent one, mainly. You can always specity the pin's symbol as a string. But
	//	the name must be equal to the specidied in the map-file one
	//
	printf("%s:%d\n\r", code, value); 
	return;
}

void keepTrack_numID(uint8_t pin, uint8_t value) {
	//
	// Description:
	//	This API has been created for all MCU that use just numeric ID to indicate a pin, it is usual in 32bit
	//	architectures (eg. ESP32). But using this you can only keep track of (GPIO) digital pins.
	//
#ifdef TARGET_AVR8
	; // It should be not used for AVR8 architecture. Use keepTrack_strID() instead.
	
#elifdef TARGET_ESP32
	printf("GPIO_NUM_%d:%d\n\r", pin, value); 
	
#else
#error "ERROR! TARGET_<ARCH> has not been defined or it is an unknown one"
#endif
	return;
}
