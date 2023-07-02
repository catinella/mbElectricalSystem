/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbElectricalSystem.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This file contains all software needed by the ATmega16 to manage your motorbike's services (eg. start, stop, lights...)
//	In order to build the binary file I used XC-8 compiler, you can download it by the Microchip's website
//
//	[!] The Makefile has been created by MP-Lab-X (by Microchip) automatically. So, it is not exactly human readable
//
//
//	Symbol description:
//		+---------------+------------------+-----------------------------------------------------+
//		|     Symbol    |    Plug/link     |      Description                                    |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_VX1         | resistors-key    | first voltage value                                 |
//		| i_VX2         |                  | second  "       "                                   |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_VY1         |  internal link   | first voltage reference                             |
//		| i_VY2         |                  | second  "         "                                 |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_NEUTRAL     | from gearbox     | it is 0 when the gear is in neutral position        |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_DECOMPRESS  | from decompress. | it is 0 when decompressor has been pushes           |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_KEEPALIVE   | internal only    | it keeps the system on when you unplug the key      |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_LEFTARROW   |                  | left turn blinking indicator switch                 |
//		| i_DOWNLIGHT   |                  | low beam command                                    |
//		| i_UPLIGHT     | left controls    | dazzling beam command                               |
//		| i_RIGHTARROW  |                  | right turn blinking indicator switch                |
//		| i_HORN        |                  | horn button                                         |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_BIKESTAND   | from bikestand   | it is 0 when the bike is placed on the stand        |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_STARTBUTTON |                  | the tipical motorbike engine start button           |
//		| i_BRAKESWITCH | right controls   | the switch on the front brake command               |
//		| i_ENGINEON    |                  | the on/off engine switch in on-position             |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_ADDLIGHT    | from ext. switch | additional high power light                         |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_LIGHTONOFF  | from ext. switch | it turns on the normal lights (low/dazzling beam)   |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_STARTENGINE | rear mtb. side   | it activates the (NO) engine-relay                  |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_ENGINEREADY |                  |                                                     |
//		| o_NEUTRAL     |                  |                                                     |
//		| o_RIGHTARROW  |   cockpit LED    |                                                     |
//		| o_LEFTARROW   | indicators and   |                                                     |
//		| o_DOWNLIGHT   |    services      |                                                     |
//		| o_UPLIGHT     |                  |                                                     |
//		| o_ADDLIGHT    |                  |                                                     |
//		| o_HORN        |                  |                                                     |
//		+---------------+------------------+-----------------------------------------------------+
//
//
//		Settings:
//		=========
//		BLINK_DELAY     4000000
//		V_TOLERANCE     2       // 1 = 19mV
//		BUTTON_DEBOUNC  10000
//
// License:
//	KiCad Schematics distributed under the GPL-3 License.
//
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
#include "mbesUtilities.h"
#include "mbesSerialConsole.h"

//
// PINs declaration
//
#define i_VX1          "A0"
#define i_VX2          "A1"
#define i_VY1          "A2"
#define i_VY2          "A3"
// ==== available ==== "A4"
// ==== available ==== "A5"
#define o_ENGINEOFF    "A6"
#define i_NEUTRAL      "A7"

#define i_LEFTARROW    "B0"
#define i_DOWNLIGHT    "B1"
#define i_UPLIGHT      "B2"
#define i_RIGHTARROW   "B3"
#define i_HORN         "B4"
#define i_BIKESTAND    "B5"
#define o_ENGINEREADY  "B6"
#define o_NEUTRAL      "B7"

// ==== available ==== "C0"
#define i_STARTBUTTON  "C1"
#define i_ENGINEON     "C2"
#define i_DECOMPRESS   "C3"
#define o_RIGHTARROW   "C4"
#define o_LEFTARROW    "C5"
#define i_ADDLIGHT     "C6"
#define i_LIGHTONOFF   "C7"

// === RXD UART ===    "D0"
// === TXD UART ===    "D1"
#define o_DOWNLIGHT    "D2"
#define o_UPLIGHT      "D3"
#define o_ADDLIGHT     "D4"
#define o_HORN         "D5"
#define o_KEEPALIVE    "D6"
#define o_STARTENGINE  "D7"


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
	//	It selects the argument defined channel and converts the voltage analog-value on that channel
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
	_codeConverter(code, NULL, &pinNumber);

	if (pinNumber < ACHANS_NUMBER) {
		ADMUX &= 0x20;                 // ADMUX register initialization
		ADMUX |= pinNumber;              // Analog channel selection
	
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

	_codeConverter(code, &port, &pinNumber);

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

//-------------------------------------------------------------------------------------------------------------------------------
//                                                      M A I N
//-------------------------------------------------------------------------------------------------------------------------------

void main(void) {
	uint8_t  loop                  = 1; // It enables the main loop (Just for future applications)
	uint8_t  ready_flag            = 0; // When the flag is true (1), the motorbike is ready to accept commands
	uint8_t  decompress_flag       = 0; // It is set when the decompressor lever has been pulled
	uint8_t  engstart_flag         = 1; // It means the driver is pushing the start button and the electric engine is running
	uint16_t startButtonDebouncing = 0; // It is the counter used to manage the start button bebouncing event
	
	//
	// PINs direction setting
	//
	pinDirectionRegister(i_NEUTRAL,     INPUT);
	pinDirectionRegister(i_LEFTARROW,   INPUT);
	pinDirectionRegister(i_DOWNLIGHT,   INPUT);
	pinDirectionRegister(i_UPLIGHT,     INPUT);
	pinDirectionRegister(i_RIGHTARROW,  INPUT);
	pinDirectionRegister(i_HORN,        INPUT);
	pinDirectionRegister(i_BIKESTAND,   INPUT);
	pinDirectionRegister(i_STARTBUTTON, INPUT);
	pinDirectionRegister(i_ENGINEON,    INPUT);
	pinDirectionRegister(i_DECOMPRESS,  INPUT);
	pinDirectionRegister(i_ADDLIGHT,    INPUT);
	pinDirectionRegister(i_LIGHTONOFF,  INPUT);

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
			setPinValue(o_UPLIGHT,     !(getPinValue(i_UPLIGHT)));
			setPinValue(o_DOWNLIGHT,   !(getPinValue(i_DOWNLIGHT)));
			setPinValue(o_HORN,        !(getPinValue(i_HORN)));
			setPinValue(o_ADDLIGHT,    !(getPinValue(i_ADDLIGHT)));
			setPinValue(o_NEUTRAL,     !(getPinValue(i_NEUTRAL)));


			//
			// Blinking lights
			//
			if (getPinValue(i_LEFTARROW)  == 0) {
				setPinValue(o_LEFTARROW,  blink());
				setPinValue(o_RIGHTARROW, 0);

			} else if (getPinValue(i_RIGHTARROW) == 0) {
				setPinValue(o_RIGHTARROW, blink());
				setPinValue(o_LEFTARROW,  0);

			} else {
				setPinValue(o_RIGHTARROW, 0);
				setPinValue(o_LEFTARROW,  0);
			}
			

			if (getPinValue(i_ENGINEON) == 0) {
				//
				// "_i_ENGINEON" is enablen (0) when the switch placed in the right hand-bar is set to "RUN".
				// In different case, It stop the engine and does not allow the motorbike engine to start or
				// re-start.
				//
				
				// Disabling CDI blocking
				setPinValue(o_ENGINEOFF, 0);

				// Decompressor sensor management
				if (getPinValue(i_DECOMPRESS) == 0) decompress_flag = 1;
				
				if (
					decompress_flag &&
					getPinValue(i_NEUTRAL)     == 0 && 
					getPinValue(i_STARTBUTTON) == 0
				) {
					engstart_flag = 1;
					decompress_flag = 0;
					setPinValue(o_STARTENGINE, 1); // The electric starter motor is rounding!!
				}

			} else {
				decompress_flag = 0;             // it resets decompressor status
				setPinValue(o_ENGINEREADY, 0);   // it turns-off the led indicator
				setPinValue(o_ENGINEOFF,   1);   // it stops the motorbike engine using CDI. (paranoide option)
			}

			if (engstart_flag) {
				// The electric starter is running...
				if (getPinValue(i_STARTBUTTON) == 0)
					// The button is still pressed
					startButtonDebouncing = BUTTON_DEBOUNC;
				else
					// Probably the button has been released
					startButtonDebouncing--;

				if (startButtonDebouncing == 0) {
					// OK the start button has been released
					setPinValue(o_STARTENGINE, 0);
					engstart_flag = 0;
				}
			}
		}
	}

	return;
}
