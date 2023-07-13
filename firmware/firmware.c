/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   firmware.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This file contains all software needed by the ATmega16 to manage your motorbike's services (eg. start, stop, lights...)
//	In order to build the binary file I used XC-8 compiler, you can download it by the Microchip's website
//
//	Settings:
//		BLINK_DELAY     4000000
//		V_TOLERANCE     2       // 1 = 19mV
//		BUTTON_DEBOUNC  10000
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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <mbesUtilities.h>
#include <mbesSerialConsole.h>
#include <mbesSelector.h>
#include <mbesPinsMap.h>


#define BLINK_DELAY     4000000
#define V_TOLERANCE     10
#define BUTTON_DEBOUNC  10000
#define ACHANS_NUMBER   4

//------------------------------------------------------------------------------------------------------------------------------
//                                                 F U N C T I O N S
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

	if (pinNumber < ACHANS_NUMBER) {
		ADMUX &= 0x20;                 // ADMUX register initialization
		ADMUX |= pinNumber;              // Analog channel _selection
	
		ADCSRA |= (1 << ADSC);         // Convertion starting...

		while (ADCSRA & (1 << ADSC));  // Waiting for convertion operation
	}

	return ADC;
}


uint8_t blink() {
	//
	// Description:
	//	It returns 1 or 0 alternating after a given time
	//
	static uint8_t  status = 0;
	static uint32_t counter = 0;

	if (counter >= BLINK_DELAY) {
		if (status == 0) status = 1;
		else             status = 0;
		counter = 0;
	} else
		counter++;

	return(status);
}


void setPinValue (const char *code, uint8_t value) {
	//
	// Description:
	//	It set the argument defined boolean value to the output pin
	//
	char    port;
	uint8_t pinNumber;

	codeConverter(code, &port, &pinNumber);

	if      (port == 'A' && value) PORTA |=  (1 << pinNumber);
	if      (port == 'A')          PORTA &= ~(1 << pinNumber);
	else if (port == 'B' && value) PORTB |=  (1 << pinNumber);
	else if (port == 'B')          PORTB &= ~(1 << pinNumber);
	else if (port == 'C' && value) PORTC |=  (1 << pinNumber);
	else if (port == 'C')          PORTC &= ~(1 << pinNumber);
	else if (port == 'D' && value) PORTD |=  (1 << pinNumber);
	else if (port == 'D')          PORTD &= ~(1 << pinNumber);
	else {
		// ERROR!
	}

	return;
}

//------------------------------------------------------------------------------------------------------------------------------
//                                                      M A I N
//------------------------------------------------------------------------------------------------------------------------------

int main(void) {
	uint8_t  loop             = 1; // It enables the main loop (Just for future applications)
	uint8_t  ready_flag       = 0; // When the flag is true (1), the motorbike is ready to accept commands
	uint8_t  FSM              = 0;
	
	struct mbesSelector 
		leftArr_sel, rightArr_sel, dLight_sel, uLight_sel, horn_sel, engStart_sel, decomp_sel, addLight_sel, light_sel,
		 engOn_sel
	;
	
	//
	// PINs direction setting
	//
	pinDirectionRegister(i_NEUTRAL,     INPUT);
	pinDirectionRegister(i_BIKESTAND,   INPUT);
	pinDirectionRegister(o_ENGINEOFF,   OUTPUT);
	pinDirectionRegister(o_ENGINEREADY, OUTPUT);
	pinDirectionRegister(o_NEUTRAL,     OUTPUT);
	pinDirectionRegister(o_RIGHTARROW,  OUTPUT);
	pinDirectionRegister(o_LEFTARROW,   OUTPUT);
	pinDirectionRegister(o_DOWNLIGHT,   OUTPUT);
	pinDirectionRegister(o_UPLIGHT,     OUTPUT);
	pinDirectionRegister(o_ADDLIGHT,    OUTPUT);
	pinDirectionRegister(o_HORN,        OUTPUT);
	pinDirectionRegister(o_KEEPALIVE,   OUTPUT);
	pinDirectionRegister(o_STARTENGINE, OUTPUT);
	
	
	//
	// _selectors initialization
	//
	mbesSelector_init(&horn_sel,     BUTTON, i_HORN);
	mbesSelector_init(&engStart_sel, BUTTON, i_STARTBUTTON);
	mbesSelector_init(&decomp_sel,   BUTTON, i_DECOMPRESS);
	mbesSelector_init(&leftArr_sel,  SWITCH, i_LEFTARROW);
	mbesSelector_init(&dLight_sel,   SWITCH, i_DOWNLIGHT);
	mbesSelector_init(&uLight_sel,   SWITCH, i_UPLIGHT);
	mbesSelector_init(&rightArr_sel, SWITCH, i_RIGHTARROW);
	mbesSelector_init(&addLight_sel, SWITCH, i_ADDLIGHT);
	mbesSelector_init(&light_sel,    SWITCH, i_LIGHTONOFF);
	mbesSelector_init(&engOn_sel,    SWITCH, i_ENGINEON);

	

	//
	// A/D converter settings....
	//
	ADCSRA = (1 << ADEN);    // A/D converter enabling
	

	//
	// Starting conditions
	//
	setPinValue(o_RIGHTARROW,  0);
	setPinValue(o_LEFTARROW,   0);
	setPinValue(o_DOWNLIGHT,   0);
	setPinValue(o_UPLIGHT,     0);
	setPinValue(o_ADDLIGHT,    0);
	setPinValue(o_HORN,        0);
	setPinValue(o_KEEPALIVE,   0); // IMPORTANT!!
	setPinValue(o_STARTENGINE, 0);
	setPinValue(o_ENGINEOFF,   1);


	// USART port initialization
	USART_Init(9600);


	while (loop) {
		if (ready_flag == 0) {
			//
			// Resistor keys evaluation
			//
			if (
				abs(ADC_read(i_VX1) - ADC_read(i_VY1)) < V_TOLERANCE &&
				abs(ADC_read(i_VX2) - ADC_read(i_VY2)) < V_TOLERANCE 
			) {
				// The keyword has been authenicated, you can unplug it
				setPinValue(o_KEEPALIVE, 1);
				ready_flag = 1;

			} else {
				// Waiting (1ms) to prevent brutal-force attack and for analog circuit re-initialization
			}
		
		} else {
			//
			// Lights and horn
			//
			if (mbesSelector_get(light_sel)) {
				setPinValue(o_DOWNLIGHT, mbesSelector_get(dLight_sel));
				setPinValue(o_UPLIGHT,   mbesSelector_get(uLight_sel));
			}
			setPinValue(o_HORN,     mbesSelector_get(horn_sel));
			setPinValue(o_ADDLIGHT, mbesSelector_get(addLight_sel));
			setPinValue(o_NEUTRAL,  !(getPinValue(i_NEUTRAL)));


			//
			// Blinking lights
			//
			if (mbesSelector_get(leftArr_sel)) {
				setPinValue(o_LEFTARROW,  blink());
				setPinValue(o_RIGHTARROW, 0);

			} else if (mbesSelector_get(rightArr_sel)) {
				setPinValue(o_RIGHTARROW, blink());
				setPinValue(o_LEFTARROW,  0);

			} else {
				setPinValue(o_RIGHTARROW, 0);
				setPinValue(o_LEFTARROW,  0);
			}
			
				
			// Decompressor sensor management
			if (FSM == 1) {
				if (mbesSelector_get(engOn_sel)) { 
					setPinValue(o_ENGINEOFF, 0);
					if (mbesSelector_get(decomp_sel)) FSM = 2;
				} else
					setPinValue(o_ENGINEOFF, 1); // 1 means eng locked

				
			// Electric starter engine starting...
			} else if (FSM == 2) {
				if (mbesSelector_get(engOn_sel) == false) 
					FSM = 1;
					
				else if (getPinValue(i_NEUTRAL) == 0) {
					 // The electric starter motor is rounding!!
					 if (mbesSelector_get(engStart_sel)) {
						setPinValue(o_STARTENGINE, 1);
						FSM = 3;
					}
				} else
					// Electric starter engine off
					setPinValue(o_STARTENGINE, 1);


			// Electric starter engine stopping...
			} else if (FSM == 3) {
				// EngOn == false
				if (mbesSelector_get(engOn_sel) == false || mbesSelector_get(engStart_sel) == false) {
					FSM = 1;
					setPinValue(o_STARTENGINE, 0);
				}	
			}
		}
	}

	return(0);
}
