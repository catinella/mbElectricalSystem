
/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   test06.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This simple test has been written to verify the iInputInterface module and requires the following circuit.
//	The button-A (i_CONF1) has been configured as simple button, so everytime you will push it, the led-A (o_KEEPALIVE pin)
//	will be on. Instead, the button-B (i_CONF2 pin) is configured ad HOLD-ON button. So, it will require a click to turn-on
//	the led-B (o_NEUTRAL pin), and another click to turn-off the led. The switch (i_UPLIGHT pin) functions like a simple
//	button, but in the console you should see there is no disable time for debouncing
//
//	Vcc --------------+------+-------+-----+------------------------------------------
//	                  |      |       |     |
//	                +-+-+  +-+-+   +-+-+   |  +-------+    +---------+
//	                |10K|  |10K|   |10K|   |  |       |    |   USB   |       
//	                |OHM|  |OHM|   |OHM|   +--+       +--->| Console |
//	                +-+-+  +-+-+   +-+-+      |       |    +---------+
//	          _--_    |      |       |        |       +---[LED B]-------------------+    
//	       +--O  O----+-----------------------+       |                             |
//	       |(button-A)       |       |        |       +---[LED A]------------+      |
//	       |    _==_         |       |        |  MCU  |                      |      |
//	       +----O  O---------+----------------+       +---[LED C]-----+      |      |
//	       |  (button-B)             |        |       |               |      |      |
//	       +--------------o/ o-------+--------+       |             +-+-+  +-+-+  +-+-+
//	       |            (switch)              |       |             |2K |  |2K |  |2K |
//	       |                               +--+       |             |OHM|  |OHM|  |OHM|
//	       |                               |  |       |             +-+-+  +-+-+  +-+-+
//	       |                               |  +-------+               |      |      |
//	GND ---+-------------------------------+--------------------------+------+------+---

//	This software has been developed for ESP-IDF v5.4 and ESP32-S2-DevKitM-1
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

// Platform dependent libs
#include "esp_timer.h"
#include "driver/gpio.h"

// Higher level libs
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>

// Project's libs
#include <mbesPinsMap.h>
#include <iInputInterface.h>
#include <werror.h>


void app_main(void) {
	
	
	if (iInputInterface_init() != WERRCODE_SUCCESS)
		// ERRPR!
		ESP_LOGE(__FUNCTION__, "iInputInterface module initialization failed");
		
	else {
		uint8_t       btnA = 0, btnB = 0, swC = 0, ea = 0;
		gpio_config_t ledA, ledB, ledC;
		
		//
		// PIN configuration
		//
		ledA.intr_type    = GPIO_INTR_DISABLE;
		ledA.mode         = GPIO_MODE_OUTPUT;
		ledA.pin_bit_mask = (1ULL << o_KEEPALIVE);
		ledA.pull_down_en = GPIO_PULLDOWN_DISABLE;
		ledA.pull_up_en   = GPIO_PULLDOWN_DISABLE;

		ledB = ledA;
		ledB.pin_bit_mask = (1ULL << o_NEUTRAL);
		
		ledC = ledA;
		ledC.pin_bit_mask = (1ULL << o_ENGINEREADY);
		
		if (
			gpio_config(&ledA) != ESP_OK || 
			gpio_config(&ledB) != ESP_OK ||
			gpio_config(&ledC) != ESP_OK
		)
			// ERROR!
			ESP_LOGE(__FUNCTION__, "ERROR! LED GPIO configuration failed");
		
		else if (	
			iInputInterface_new(&btnA, BUTTON,     i_CONF1)   != WERRCODE_SUCCESS ||
			iInputInterface_new(&btnB, HOLDBUTTON, i_CONF2)   != WERRCODE_SUCCESS ||
			iInputInterface_new(&swC,  SWITCH,     i_UPLIGHT) != WERRCODE_SUCCESS 

		)
			// ERRPR!
			ESP_LOGE(__FUNCTION__, "Object creation failed");
		
		else {
			bool statA = false, statB = false, statC = false;
			bool loop = true;
			ESP_LOGI(__FUNCTION__, "The library has been correctly initialized");
	
			//
			// Hardware led test
			//
			{
				gpio_num_t tmp[3] = {o_KEEPALIVE, o_NEUTRAL, o_ENGINEREADY};
				for (uint8_t t=0; t<5; t++) {
					for (uint8_t x=0; x<3; x++) {
						gpio_set_level(tmp[x], 1);
						vTaskDelay(100 / portTICK_PERIOD_MS);
						gpio_set_level(tmp[x], 0);
					}
				}
			}
			
			while (loop) {
				if (
					(ea = iInputInterface_get(btnA, &statA)) != WERRCODE_SUCCESS ||
					(ea = iInputInterface_get(btnB, &statB)) != WERRCODE_SUCCESS ||
					(ea = iInputInterface_get(swC,  &statC)) != WERRCODE_SUCCESS 
				) {
					ESP_LOGE(__FUNCTION__, "ERROR! The specified ID is not a valid one");
					loop = false;
					
				} else {
					gpio_set_level(o_KEEPALIVE,   statA ? 1 : 0);
					gpio_set_level(o_NEUTRAL,     statB ? 1 : 0);
					gpio_set_level(o_ENGINEREADY, statC ? 1 : 0);
				}
				
				vTaskDelay(50 / portTICK_PERIOD_MS);
			}
		}
	}
}
