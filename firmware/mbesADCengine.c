/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesADCengine.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	Thif file contains useful functions to convert analog inputs in digital numbers
//	Because the internal v-ref will be not used, the engine is automaticaly initialized by the ADC_read() function, in
//	transparent way.
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

#include <avr/io.h>
#include <util/twi.h>

#include <mbesADCengine.h>

#include <stdio.h>

#ifndef MBES_ADCENGINE_DEBUG
#define MBES_ADCENGINE_DEBUG 0
#endif


#if MBES_ADCENGINE_DEBUG > 0
#define LOGERR    logMsg(PSTR("ERROR! in %s(%d)"), __FUNCTION__, __LINE__);
#define LOGMSG(X) logMsg(PSTR(X));
#else
#define LOGERR    ;
#define LOGMSG(X) ;
#endif


static uint8_t initFlag = 0;

static void ADC_initialization() {
	//
	// A/D converter enabling
	//
	ADCSRA = (1 << ADEN);
	LOGMSG("ADC engine initialized");

}

//------------------------------------------------------------------------------------------------------------------------------
//                                           P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

uint16_t ADC_read (const char *code) {
	//
	// Description:
	//	It _selects the argument defined channel and converts the voltage analog-value on that channel
	//
	//	ADMUX register:
	//		+-------+-------+-------+------+------+------+------+------+
	//		| REFS1 | REFS0 | ADLAR | MUX4 | MUX3 | MUX2 | MUX1 | MUX0 |
	//		+-------+-------+-------+------+------+------+------+------+
	//		|   0   |   0   |   1   |   0  |   0  |   0  |   0  |   0  |  Reset
	//		+-------+-------+-------+------+------+------+------+------+
	//		REFS1==0 & REFS0==0 ---> external volt ref
	//		ADLAR==1            ---> left giustified result
	//
	uint8_t pinNumber;
	codeConverter(code, NULL, &pinNumber);

	if (initFlag == 0) {
		ADC_initialization();
		initFlag = 1;
	}

	if (pinNumber < ACHANS_NUMBER) {
		ADMUX &= 0x20;                 // ADMUX register initialization
		ADMUX |= pinNumber;              // Analog channel _selection
	
		ADCSRA |= (1 << ADSC);         // Convertion starting...

		while (ADCSRA & (1 << ADSC));  // Waiting for convertion operation
	}

	return ADC;
}

