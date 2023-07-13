/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   firmware-test.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This is a very simple test for the hardware component (or the simulator you are using). It just set the keep-alive PIN
//	to 5V.
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
#include <mbesPinsMap.h>
#include <avr/io.h>


int main() {
	char    *pin = o_KEEPALIVE;
	char    port = pin[0];
	uint8_t pinNumber = pin[1] - '0';
	
	while (1) {
		if      (port == 'A')  {DDRA |=  (1 << pinNumber); PORTA |=  (1 << pinNumber);}
		else if (port == 'B')  {DDRB |=  (1 << pinNumber); PORTB |=  (1 << pinNumber);}
		else if (port == 'C')  {DDRC |=  (1 << pinNumber); PORTC |=  (1 << pinNumber);}
		else if (port == 'D')  {DDRD |=  (1 << pinNumber); PORTD |=  (1 << pinNumber);}
	}
	
	return(0);
}
