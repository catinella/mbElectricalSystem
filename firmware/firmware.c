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
//
// C standard libraries
//
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


//
// Progect's sub-modules
//
#include <mbesHwConfig.h>
#include <mbesPinsMap.h>
#include <mbesUtilities.h>
#include <mbesSerialConsole.h>
#include <mbesADCengine.h>
#include <mbesMCP23008.h>
#include <mbesSelector.h>


//
// AVR Libraries
//
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>


// Settings
#define BLINK_DELAY  3

#define V_TOLERANCE  10


#if DEBUG > 0
#define LOGMSG(X)   USART_writeString(PSTR(X), USART_FLASH);
#else
#define LOGMSG(X)   ;
#endif


typedef enum {
	RKEY_EVALUATION   = 0,
	MPC23008_INIT     = 1,
	PINS_SETTING      = 2,
	SELECTORS_SETTING = 3,
	VALUE_RESTORING   = 4,
	MAIN_LOOP         = 5,
	I2CBUS_RESET      = 6,
	I2CBUS_READ       = 7,
	I2CBUS_WRITE      = 8,
	PARCKING_STATUS   = 9
}  fsmStates_t;

typedef enum {
	MTB_STOPPED_ST,
	MTB_WFR1_ST,
	MTB_WFR2_ST,
	MTB_ELSTARTING_ST,
	MTB_RUNNIG_ST
} mtbStates_t;

//------------------------------------------------------------------------------------------------------------------------------
//                                                 F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

uint8_t blink (bool reset) {
	//
	// Description:
	//	It returns 1 or 0 alternating after a given time
	//
	static uint8_t  status = 1;   // It starts with the indicator light set to ON
	static uint32_t counter = 0;

	/*
	if (status) {
		LOGMSG("BLINK-ON\n\r");
	} else {
		LOGMSG("BLINK-OFF\n\r");
	}
	*/
	
	if (reset)
		counter = 0;
	
	else if (counter >= BLINK_DELAY) {
		status = status ? 0 : 1;
		counter = 0;
	} else
		counter++;

	return(status);
}


//------------------------------------------------------------------------------------------------------------------------------
//                                                      M A I N
//------------------------------------------------------------------------------------------------------------------------------

int main(void) {
	bool        loop         = true;              // It enables the main loop (Just for future applications)
	bool        decompPushed = false;             // Flag true, means motorbike is ready to accept start commands
	fsmStates_t FSM          = RKEY_EVALUATION;
	mtbStates_t mtbState     = MTB_STOPPED_ST;
	bool        firstRound   = true;
	uint8_t     neutralPin   = 1;
	uint8_t     bikeStandPin = 1;
	uint8_t     clutchPin    = 1;
	
	struct mbesSelector 
		leftArr_sel, rightArr_sel, uLight_sel, horn_sel, engStart_sel, decomp_sel, addLight_sel, light_sel, engOn_sel;
	
	// USART port initialization
	USART_Init(RS232_BPS);

	//
	// Critic output initialization (these pins MUST be MCU's pins)
	//
	pinDirectionRegister(o_KEEPALIVE,   OUTPUT);
	pinDirectionRegister(o_STARTENGINE, OUTPUT);
	pinDirectionRegister(o_ENGINEON,    OUTPUT);
	pinDirectionRegister(o_MCPDEVRESET, OUTPUT);
	setPinValue(o_KEEPALIVE,   0);
	setPinValue(o_STARTENGINE, 0);
	setPinValue(o_ENGINEON,    0);
	setPinValue(o_MCPDEVRESET, 1); 


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
				pinDirectionRegister(i_CLUTCH,      INPUT)  &&
				pinDirectionRegister(o_ENGINEREADY, OUTPUT) &&  // It is just the LED indicator
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
					getPinsFromI2C()              &&
					setPinValue(o_RIGHTARROW,  0) &&
					setPinValue(o_LEFTARROW,   0) &&
					setPinValue(o_DOWNLIGHT,   0) &&
					setPinValue(o_UPLIGHT,     0) &&
					setPinValue(o_ADDLIGHT,    0) &&
					setPinValue(o_HORN,        0) &&
					setPinsToI2C()
				) {
					firstRound = false;
					FSM = MAIN_LOOP;
					LOGMSG("[ OK ] Output-PINs have been set to default values\n\r")
				} else {
					// ERROR!
					LOGMSG("ERROR! Default values setting step failed\n\r")
					FSM = I2CBUS_RESET;
				}

			} else {
				if (restore_MCP23008())
					FSM = I2CBUS_READ;
				else {
					// ERROR!
					LOGMSG("ERROR! Old values cannot be restored\n\r")
					FSM = I2CBUS_RESET;
				}

			}

		
		} else if (FSM == I2CBUS_READ) {
			if (getPinsFromI2C())
				FSM = MAIN_LOOP;
			else {
				// ERROR!
				LOGMSG("ERROR! I cannot read data from I2C bus\n\r");
				FSM = I2CBUS_RESET;
			}


		} else if (FSM == MAIN_LOOP) {

			// [!] the following PINs are critic ones, and they should NEVER been linked to the I/O extender.
			//     So, their function's error code has no meaning
			getPinValue(i_NEUTRAL,   &neutralPin);
			getPinValue(i_BYKESTAND, &bikeStandPin);
			getPinValue(i_CLUTCH,    &clutchPin);


			//
			// Lights
			//
			if (mbesSelector_get(light_sel)) {
				setPinValue(o_DOWNLIGHT, 1);
				setPinValue(o_UPLIGHT,  mbesSelector_get(uLight_sel));
				setPinValue(o_ADDLIGHT, mbesSelector_get(addLight_sel));
			} else {
				setPinValue(o_DOWNLIGHT, 0);
				setPinValue(o_UPLIGHT,   0);
				setPinValue(o_ADDLIGHT,  0);
			}

			// Horn
			setPinValue(o_HORN, mbesSelector_get(horn_sel));

			// NEUTRAL LED indicator
			setPinValue(o_NEUTRAL, (neutralPin == 1 ? 0 : 1));


			//
			// Blinking lights
			//
			if (mbesSelector_get(leftArr_sel)) {
				setPinValue(o_LEFTARROW,  blink(false));
				setPinValue(o_RIGHTARROW, 0);

			} else if (mbesSelector_get(rightArr_sel)) {
				setPinValue(o_RIGHTARROW, blink(false));
				setPinValue(o_LEFTARROW,  0);

			} else {
				setPinValue(o_RIGHTARROW, 0);
				setPinValue(o_LEFTARROW,  0);
				blink(true);
				blink(true);
			}
			
			
			// Decompressor sensor management
			if (mbesSelector_get(decomp_sel)) decompPushed = true;
			
			
			//
			// SECURITY policy #1
			// 	Protection by motorcycle stand down while the vehicle is running
			//
			if (neutralPin == 1 && bikeStandPin == 1) {
				if (mtbState != MTB_STOPPED_ST)
					LOGMSG("WARNING! bike stand is down!!\n\r");
				mtbState = MTB_STOPPED_ST;
			}
			
			// 
			// Master rule #1
			//	It the engine-on button is disabled then the mtb's engine must be stopped and locked, always
			//
			if (mbesSelector_get(engOn_sel) == 0) {
				mtbState = MTB_STOPPED_ST;
				setPinValue(o_ENGINEON, 0);            // Engine locked by CDI
			}

			//
			// Master rule #2
			//	If the start button is not pressed the electric eng must be stopped, in any situation
			//
			if (mbesSelector_get(engStart_sel) == false)   // i_STARTBUTTON
				setPinValue(o_STARTENGINE, 0);


			switch (mtbState) {
				case MTB_STOPPED_ST:
					//
					// In this state the mtb is stopped and it CANNOT be started by the electric engine and manually too
					//
					LOGMSG("MTB_STOPPED_ST\n\r");
					setPinValue(o_ENGINEON,    0);   // Engine locked by CDI
					setPinValue(o_ENGINEREADY, 0);   // LED: mtb is not yet ready to start
					setPinValue(o_STARTENGINE, 0);
					
					if ((neutralPin == 0 || clutchPin == 0) && decompPushed && mbesSelector_get(engOn_sel))
						mtbState = MTB_WFR1_ST;

				break;
			

				case MTB_WFR1_ST:
					//
					// In tis state driver is preparing the mtb to be started with electric eng. However he CAN
					// start it manually
					//
					setPinValue(o_ENGINEON, 1);               // Engine no more locked by CDI
					if (mbesSelector_get(decomp_sel) == 0)
						mtbState = MTB_WFR1_ST;
					else if (neutralPin == 1 && clutchPin == 1) {
						decompPushed = false;               // The mtb has been started manually
						mtbState = MTB_RUNNIG_ST;
						LOGMSG("MTB started manually\n\r");
					}
				break;


				case MTB_WFR2_ST:
					//
					// In tis state driver can choise to start the mtb using the electric eng or manually
					//
					if (neutralPin == 1 && clutchPin == 1) {
						decompPushed = false;                    // The mtb has been started manually
						mtbState = MTB_RUNNIG_ST;
						LOGMSG("MTB started manually\n\r");
					
					} else if (mbesSelector_get(engStart_sel) == true) {  // i_STARTBUTTON
						setPinValue(o_STARTENGINE, 1);
						setPinValue(o_ENGINEREADY, 0);
						LOGMSG("OK electric starter is running\n\r");
						decompPushed = false;
						mtbState = MTB_ELSTARTING_ST;
					}
				break;
			

				case MTB_ELSTARTING_ST:
					//
					// The electric start engine is running
					//
					if (mbesSelector_get(engStart_sel) == false) {  // i_STARTBUTTON
						mtbState = MTB_RUNNIG_ST;
						setPinValue(o_STARTENGINE, 0);
					}
				break;


				case MTB_RUNNIG_ST:
					//
					// The motorbike's engine is running....
					// Unfortunately, because the MCU does not know the real eng status (by RPM signal), to come back
					// in the MTB_STOPPED_ST status, the driver MUST set the engine-on switch to off
					//
					setPinValue(o_ENGINEREADY, 0);
				break;
			}

/*
			// Parcking mode
			if (
				mbesSelector_get(engOn_sel)  == false &&
				mbesSelector_get(light_sel)  == false &&
				mbesSelector_get(uLight_sel) == false
			) 
				FSM = PARCKING_STATUS;
*/
			
			// mbesSelector items updating....
			mbesSelector_update(NULL);

			FSM = I2CBUS_WRITE;
			
			
		} else if (FSM == I2CBUS_WRITE) {
			if (setPinsToI2C())
				FSM = I2CBUS_READ;
			else {
				// ERROR!
				LOGMSG("ERROR! I cannot write data in the MCP2308 device\n\r");
				FSM = I2CBUS_RESET;
			}


		} else if (FSM == PARCKING_STATUS) {
			//
			// Parcking status
			//
			LOGMSG("Parking mode\n\r");
			setPinValue(o_LEFTARROW,  blink(false));
			setPinValue(o_RIGHTARROW, blink(false));
			setPinValue(o_DOWNLIGHT,  1);

			// [!] The lonely way to exit by the parcking state, is to turning off the motorbike
			
		} else if (FSM == I2CBUS_RESET) {
			setPinValue(o_MCPDEVRESET, 0); 
			LOGMSG("WARNING! MCP2308 device reset\n\r");
			_delay_ms(200);
			setPinValue(o_MCPDEVRESET, 1); 
			_delay_ms(400);
			FSM = MPC23008_INIT;
		}


		// delay
		#if DEBUG > 0
		_delay_ms(200);
		#else
		_delay_us(100);
		#endif
	}

	return(0);
}
