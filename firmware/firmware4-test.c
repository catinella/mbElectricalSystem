/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   firmware4-test.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This test has been developped to ckeck for the I/O extension chip MCP23008-XP. This device is connected to the ATmega16
//	using I2C BUS.
//
//	This test will read the MCP23008-XP's GP0 (pin-10) status and will set the GP3 (pin-13) at the same status.
//	
//	The MCP23008-XP configuration:
//		dev-address = 01000:0000 // according to the factory's address
//		IOCON       = XX10: X11X // No sequential I/O, no interrupt generation
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
#include <mbesI2C.h>

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>

//
// I2C I/O extender MCP23008XP's regirters
//
#define IODIR_ADDR 0x00
#define IOCON_ADDR 0x05
#define GPPU_ADDR  0x06
#define GPIO_ADDR  0x09

#define MC23008AP (MC23008_ADDR << 1) | 64

#define IOCON_VALUE 38

typedef enum _fsm_state {
	IOCON_select_state,
	IOCON_setting_state,
	IOCON_checking_state,
	IODIR_select_state,
	IODIR_reading_state,
	IODIR_updating_state,
	IODIR_checking_state,
	GPIO_select_state,
	GPIO_reading_state,
	GPIO_updating_state,
	STOP_state,
	BUSRESET_state,
	END
} fsm_state;

//------------------------------------------------------------------------------------------------------------------------------
//                                                    M A I N
//------------------------------------------------------------------------------------------------------------------------------
int main() {
	uint8_t   regValue = 0;
	fsm_state fsm = IODIR_select_state;
	uint8_t   status = 0;

	USART_Init(9600);
	I2C_INIT
	_delay_ms(100);
	
	USART_writeString(PSTR("************* TEST START *************\n\r"), USART_FLASH);

	while(fsm != END) {
		
		//
		// Initialization: "IOCON" register selecting
		//
		if (fsm == IOCON_select_state) {
			status = 0;
			USART_writeString(PSTR("\nICON selecting\n\r"), USART_FLASH);
			I2C_START(status)
			if (status) {
				I2C_WRITE(MC23008AP, status)
				if (status) {
					I2C_WRITE(IOCON_ADDR, status)
					if (status)	fsm = IOCON_setting_state;
				}
			}
			
			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR!I cannot select IOCON register\n\n\r"), USART_FLASH);
				fsm = BUSRESET_state;
			}


		//
		// Initialization: "IOCON" register writing
		//
		} else if (fsm == IOCON_setting_state) {
			status   = 0;
			regValue = IOCON_VALUE;
			USART_writeString(PSTR("\nICON setting\n\r"), USART_FLASH);
			I2C_START(status)
			if (status) {
				I2C_WRITE(MC23008AP, status);
				if (status == 1) {
					I2C_WRITE(regValue, status)
					if (status == 1) {
						fsm = IOCON_checking_state;
						logMsg(PSTR("IOCON: %d"), regValue);
					}
				}
			}

			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR! I cannot set the IOCON register\n\n\r"), USART_FLASH);
				fsm = BUSRESET_state;
			}



		//
		// Initialization: Checking for "IOCON" register content
		//
		} else if (fsm == IOCON_checking_state) {
			status   = 0;
			regValue = 0;
			USART_writeString(PSTR("\nChecking for ICON\n\r"), USART_FLASH);
			I2C_START(status)
			if (status) {
				I2C_WRITE(MC23008AP|1, status)
				if (status) {
					I2C_READ(I2C_NACK, regValue, status)
					if (status) {
						logMsg(PSTR("IODIR: %d"), regValue);
						if (regValue == IOCON_VALUE) fsm = IODIR_select_state;
					}
				}
			}
			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR!I cannot read IODIR register\n\n\r"), USART_FLASH);
				fsm = BUSRESET_state;
			}


		//
		// Initialization: "IODIR" register selecting
		//
		} else if (fsm == IODIR_select_state) {
			status = 0;
			USART_writeString(PSTR("\nIODIR selecting \n\r"), USART_FLASH);
			I2C_START(status)
			if (status) {
				I2C_WRITE(MC23008AP, status)
				if (status) {
					I2C_WRITE(IODIR_ADDR, status)
					if (status)	fsm = IODIR_reading_state;
				}
			}

			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR!I cannot select IODIR register\n\n\r"), USART_FLASH);
				fsm = BUSRESET_state;
			}
	

		//
		// Initialization: "IODIR" register reading
		//
		} else if (fsm == IODIR_reading_state) {
			status = 0;
			regValue = 0;
			USART_writeString(PSTR("\nIODIR reading\n\r"), USART_FLASH);
			I2C_START(status)
			if (status) {
				I2C_WRITE(MC23008AP|1, status)
				if (status) {
					I2C_READ(I2C_NACK, regValue, status)
					if (status) {
						fsm = IODIR_updating_state;
						logMsg(PSTR("IODIR: %d"), regValue);
					}
				}
			}
			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR!I cannot read IODIR register\n\n\r"), USART_FLASH);
				fsm = BUSRESET_state;
			}


		//
		// IODIR register updating
		//
		} else if (fsm == IODIR_updating_state) {
			status = 0;

			// "IODIR" register changing
			regValue &= ~(1 << 3);               // GP3 (bit0 --> OUTPUT)
			regValue |=  (1 << 0);               // GP0 (bit1 --> INPUT)
		
			// "IODIR" register saving
			USART_writeString(PSTR("\nIODIR updating\n\r"), USART_FLASH);
			I2C_START(status)
			if (status == 1) {
				I2C_WRITE(MC23008AP, status);
				if (status == 1) {
					I2C_WRITE(regValue, status)
					if (status == 1) fsm = IODIR_checking_state;
				}
			}

			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR! I cannot update the IODIR register\n\n\r"), USART_FLASH);
				fsm = BUSRESET_state;
			}

		
		//
		// Checking for IODIR register
		//
		} else if (fsm == IODIR_checking_state) {
			uint8_t tmp = 0;
			status = 0;
			USART_writeString(PSTR("\nChecking for IODIR\n\r"), USART_FLASH);
			
			I2C_START(status)
			if (status == 1) {
				I2C_WRITE(MC23008AP|1, status)
				if (status) {
					I2C_READ(I2C_NACK, tmp, status)
					if (status) {
						if (tmp == regValue) {
							fsm = GPIO_select_state;
							logMsg(PSTR("OK! IODIR register has been verified (value=%d)"), tmp);
						} else {
							logMsg(PSTR("IODIR: %d"), tmp);
							
							// ERROR!
							USART_writeString(PSTR("ERROR! IODIR register has been not updated\n\n\r"), USART_FLASH);
							fsm = BUSRESET_state;
							status = 1;
						}
					}
				} 
			}
			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR!I cannot read IODIR register\n\n\r"), USART_FLASH);
				fsm = BUSRESET_state;
			}


		//
		// GPIO register selecting
		//
		} else if (fsm == GPIO_select_state) {
			USART_writeString(PSTR("\nGPIO selecting\n\r"), USART_FLASH);
			I2C_START(status)
			if (status) {
				I2C_WRITE(MC23008AP, status)
				if (status) {
					I2C_WRITE(GPIO_ADDR, status)
					if (status) {
						USART_writeString(PSTR("OK\n\n\r"), USART_FLASH);
						fsm = GPIO_reading_state;
					}
				}
			}

			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR!\n\n\r"), USART_FLASH);
				fsm = BUSRESET_state;
			}

		//
		// GPIO register reading
		//
		} else if (fsm == GPIO_reading_state) {
			USART_writeString(PSTR("\nGPIO reading\n\r"), USART_FLASH);
			I2C_START(status)
			if (status == 1) {
				I2C_WRITE((MC23008AP|1), status)
				if (status) {
					I2C_READ(I2C_NACK, regValue, status)
					if (status) { 
						logMsg(PSTR("GPIO: %d"), regValue);
						fsm = GPIO_updating_state;
					}
				}
			}

			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR!I cannot read GPIO\n\n\r"), USART_FLASH);
				fsm = 29;
			}


		//
		// GPIO updating
		//
		} else if (fsm == GPIO_updating_state) {
			USART_writeString(PSTR("\nGPIO updating\n\r"), USART_FLASH);

			if ((regValue & 1) == 0) {
				USART_writeString(PSTR("GP0 == 0\n\r"), USART_FLASH);
				regValue &= ~(1 << 3);      // GP3 = 0
			} else {
				USART_writeString(PSTR("GP0 == 1\n\r"), USART_FLASH);
				regValue |= (1 << 3);       // GP3 = 1
			}

			// GPIO register saving
			I2C_START(status)
			if (status == 1) {
				I2C_WRITE(MC23008AP, status)
				if (status) {
					I2C_WRITE(regValue, status)
					if (status) {
						USART_writeString(PSTR("OK\n\n\r"), USART_FLASH);
						fsm = STOP_state;
					}
				}

			}

			if (status == 0) {
				// ERROR!
				USART_writeString(PSTR("ERROR!I cannot update GPIO\n\n\r"), USART_FLASH);
				fsm = 29;
			}


		//
		// Stop 
		//
		} else if (fsm == STOP_state) {
			USART_writeString(PSTR("\n=== STOP ===\n\r"), USART_FLASH);
			I2C_STOP
			fsm = GPIO_select_state;

		//
		// Error (BUS reset)
		//
		} else if (fsm == BUSRESET_state) {
			USART_writeString(PSTR("\n=== BUSRESET ===\n\r"), USART_FLASH);
			// I2C_BUSRESET
			//fsm = IODIR_select_state;
		}

		_delay_ms(1000);
	}
			

	return(0);
}
