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
// C standard libraries
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Progect's sub-modules
#include <mbesHwConfig.h>
#include <mbesPinsMap.h>
#include <mbesUtilities.h>
#include <mbesSerialConsole.h>
#include <mbesADCengine.h>
#include <mbesMCP23008.h>
#include <mbesSelector.h>

// AVR Libraries
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>


// Settings
#define BLINK_DELAY     4000000
#define V_TOLERANCE     10


#if DEBUG > 0
#define LOGMSG(X)      USART_writeString(PSTR(X), USART_FLASH);
#else
#define LOGMSG(X)      ;
#endif


typedef enum _fsmStates {
	RKEY_EVALUATION,
	MPC23008_INIT,
	PINS_SETTING,
	SELECTORS_SETTING,
	VALUE_RESTORING,
	NORMAL_STATUS,
	I2CBUS_RESET
}  fsmStates;

//------------------------------------------------------------------------------------------------------------------------------
//                                                 F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

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


//------------------------------------------------------------------------------------------------------------------------------
//                                                      M A I N
//------------------------------------------------------------------------------------------------------------------------------

int main(void) {
	bool      loop         = true;              // It enables the main loop (Just for future applications)
	bool      canStart     = false;             // When the flag is true (1), the motorbike is ready to accept commands
	fsmStates FSM          = RKEY_EVALUATION;
	bool      firstRound   = true;
	uint8_t   neutralPin   = 1;
	uint8_t   bikeStandPin = 0;
	
	struct mbesSelector 
		leftArr_sel, rightArr_sel, dLight_sel, uLight_sel, horn_sel, engStart_sel, decomp_sel, addLight_sel, light_sel,
		 engOn_sel
	;
	
	// USART port initialization
	USART_Init(9600);

	//
	// Critic output initialization (these pins MUST be MCU's pins)
	//
	pinDirectionRegister(o_KEEPALIVE,   OUTPUT);
	pinDirectionRegister(o_STARTENGINE, OUTPUT);
	pinDirectionRegister(o_ENGINEON,    OUTPUT);
	setPinValue(o_KEEPALIVE,   0);
	setPinValue(o_STARTENGINE, 0);
	setPinValue(o_ENGINEON,    0);

	while (loop) {

		if (FSM == RKEY_EVALUATION) {
			//
			// Resistor keys evaluation
			//
			if (
				abs(ADC_read(i_VX1) - ADC_read(i_VY1)) < V_TOLERANCE &&
				abs(ADC_read(i_VX2) - ADC_read(i_VY2)) < V_TOLERANCE 
			) {
				// The keyword has been authenicated, you can unplug it
				setPinValue(o_KEEPALIVE, 1);
				FSM = MPC23008_INIT;
				LOGMSG("[ OK ] key has been accepted\n\r")
			} 

			// [!] The following delay is used to prevent brutal-force attack (when ready_flag == 0) and to allow
			//    the MCP23008 to boot
			_delay_ms(100);

			
		//
		// MCP23008 Initialization
		//
		} else if (FSM == MPC23008_INIT) {
			if (init_MCP23008(MCP23008_ADDR)) {
				FSM = PINS_SETTING;
				LOGMSG("[ OK ] MCP23008 initialized\n\r")
			} else {
				// ERROR!
				LOGMSG("ERROR! MCP23008 initialization step failed\n\r")
				FSM = I2CBUS_RESET;
			}


		//
		// PINs direction setting
		//
		} else if (FSM == PINS_SETTING) {
			if (
				pinDirectionRegister(i_NEUTRAL,     INPUT)  &&
				pinDirectionRegister(i_BYKESTAND,   INPUT)  &&
				pinDirectionRegister(o_ENGINEREADY, OUTPUT) &&
				pinDirectionRegister(o_NEUTRAL,     OUTPUT) &&
				pinDirectionRegister(o_RIGHTARROW,  OUTPUT) &&
				pinDirectionRegister(o_LEFTARROW,   OUTPUT) &&
				pinDirectionRegister(o_DOWNLIGHT,   OUTPUT) &&
				pinDirectionRegister(o_UPLIGHT,     OUTPUT) &&
				pinDirectionRegister(o_ADDLIGHT,    OUTPUT) &&
				pinDirectionRegister(o_HORN,        OUTPUT)
			) {
				FSM = SELECTORS_SETTING;
				LOGMSG("[ OK ] PINs initialized\n\r")
			} else {
				// ERROR!
				LOGMSG("ERROR! PINs direction setting step failed\n\r")
				FSM = I2CBUS_RESET;
			}


		//
		// selectors initialization
		//
		} else if (FSM == SELECTORS_SETTING) {
			if (
				mbesSelector_init(&horn_sel,     BUTTON, i_HORN)        &&
				mbesSelector_init(&engStart_sel, BUTTON, i_STARTBUTTON) &&
				mbesSelector_init(&decomp_sel,   BUTTON, i_DECOMPRESS)  &&
				mbesSelector_init(&leftArr_sel,  SWITCH, i_LEFTARROW)   &&
				mbesSelector_init(&dLight_sel,   SWITCH, i_DOWNLIGHT)   &&
				mbesSelector_init(&uLight_sel,   SWITCH, i_UPLIGHT)     &&
				mbesSelector_init(&rightArr_sel, SWITCH, i_RIGHTARROW)  &&
				mbesSelector_init(&addLight_sel, SWITCH, i_ADDLIGHT)    &&
				mbesSelector_init(&light_sel,    SWITCH, i_LIGHTONOFF)  &&
				mbesSelector_init(&engOn_sel,    SWITCH, i_ENGINEON)
			) {
				FSM = VALUE_RESTORING;
				LOGMSG("[ OK ] Selectors initialized\n\r")
			} else {
				// ERROR!
				LOGMSG("ERROR! selectors initialization step failed\n\r")
				FSM = I2CBUS_RESET;
			}


		//
		// Old/Default values setting
		//
		} else if (FSM == VALUE_RESTORING) {
			if (firstRound) {
				if (
					setPinValue(o_RIGHTARROW,  0) &&
					setPinValue(o_LEFTARROW,   0) &&
					setPinValue(o_DOWNLIGHT,   0) &&
					setPinValue(o_UPLIGHT,     0) &&
					setPinValue(o_ADDLIGHT,    0) &&
					setPinValue(o_HORN,        0)
				) {
					firstRound = false;
					FSM = NORMAL_STATUS;
					LOGMSG("[ OK ] Output-PINs have been set to default values\n\r")
				} else {
					// ERROR!
					LOGMSG("ERROR! Default values setting step failed\n\r")
					FSM = I2CBUS_RESET;
				}

			} else {
				if (restore_MCP23008())
					FSM = NORMAL_STATUS;
				else {
					// ERROR!
					LOGMSG("ERROR! Old values cannot be restored\n\r")
					FSM = I2CBUS_RESET;
				}

			}

		
		} else if (FSM == NORMAL_STATUS) {

			// [!] the following PINs are critic ones, and they should NEVER been linked to the I/O extender.
			//     So, their function's error code has no meaning
			getPinValue(i_NEUTRAL,   &neutralPin);
			getPinValue(i_BYKESTAND, &bikeStandPin);


			//
			// Lights and horn
			//
			if (mbesSelector_get(light_sel)) {
				setPinValue(o_DOWNLIGHT, mbesSelector_get(dLight_sel));
				setPinValue(o_UPLIGHT,   mbesSelector_get(uLight_sel));
			}
			setPinValue(o_HORN,     mbesSelector_get(horn_sel));
			setPinValue(o_ADDLIGHT, mbesSelector_get(addLight_sel));
			setPinValue(o_NEUTRAL,  (neutralPin == 1 ? 0 : 1));


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
			
			
			//
			// Protection by motorcycle stand down when the vehicle is running
			//
			if (neutralPin == 1 && bikeStandPin == 1) {
				setPinValue(o_ENGINEON, 0);   // Engine locked by CDI
				canStart = false;


			//	
			// Decompressor sensor management
			//	
			} else if (neutralPin == 0) {
				
				if (mbesSelector_get(decomp_sel)) {

					// This LED inform the biker the engine is ready to start
					setPinValue(o_ENGINEREADY, 1);

					canStart = true;
				}

				if (canStart && mbesSelector_get(engOn_sel)) {
					// Decompressor MUST be released
					if (mbesSelector_get(decomp_sel)) {
						setPinValue(o_ENGINEON, 0); // 0 means eng locked
					} else {
						// *** START ***
						setPinValue(o_ENGINEON,    1);
						setPinValue(o_ENGINEREADY, 0);
						canStart = false;
					}
				}

			} else
				// Just to be paranoide
				canStart = false;

				
			if (mbesSelector_get(engOn_sel) == 0)
				// STOP the electric starter engine
				setPinValue(o_ENGINEON, 0);
		}

		// delay
		#if DEBUG > 0
		_delay_ms(5000);
		#endif
	}

	return(0);
}
