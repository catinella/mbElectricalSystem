/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   test03.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This test use a GPIO to turn on/off a connected led
//	
//	5V ---------------+-----+------------------------------------------
//	                  |     |
//	                +-+-+   |  +-------+
//	                |10K|   +--+       |        +---------+
//	                |OHM|      |       |        |         |
//	                +-+-+      |       +------->|   USB   |
//	         _--_     |        |       |        | Console |         
//	     +---O  O-----+--------+  MCU  |        |         |
//	     |                     |       |        +---------+
//	     |                     |       |       
//	     |                     |       |              +--------+
//	     |                  +--+       +-------[LED]--+ 2K Ohm +---+
//	     |                  |  +-------+              +--------+   |
//	     |                  |                                      |
//	GND -+------------------+--------------------------------------+---
//
//	This software has been developed for ESP-IDF v5.4 and ESP32-S2-DevKitM-1
//
// License:
//	Copyright (C) 2023 Silvano Catinella <catinella@yahoo.com>
//
//	This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
//	License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
//	version.
//
//
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

#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include "esp_err.h"
#include "driver/gpio.h"

#include "mbesPinsMap.h"

	

void app_main(void) {
	
	gpio_config_t inputPin, outputPin;
	int           level;
	
	inputPin.intr_type    = GPIO_INTR_DISABLE;
	inputPin.mode         = GPIO_MODE_INPUT;
	inputPin.pin_bit_mask = (1ULL << i_CONF1);
	inputPin.pull_down_en = GPIO_PULLDOWN_DISABLE;
	inputPin.pull_up_en   = GPIO_PULLUP_DISABLE;

	outputPin.intr_type    = GPIO_INTR_DISABLE;
	outputPin.mode         = GPIO_MODE_OUTPUT; 
	outputPin.pin_bit_mask = (1ULL << o_KEEPALIVE);
	outputPin.pull_down_en = GPIO_PULLDOWN_DISABLE;
	outputPin.pull_up_en   = GPIO_PULLUP_DISABLE;
	
	if (gpio_config(&inputPin) == ESP_OK && gpio_config(&outputPin) == ESP_OK) {
		ESP_LOGI("-->", "OK! All PINs have been correctly configured");
		
		while (1) {
			level = gpio_get_level(i_CONF1);
			ESP_LOGI("-->", "%d", level);
			if (level)
				gpio_set_level(o_KEEPALIVE, 1);
			else
				gpio_set_level(o_KEEPALIVE, 0);
				
			vTaskDelay(1 / portTICK_PERIOD_MS); // Delay di 500ms
		}
	} else {
		// ERROR!
		ESP_LOGE("-->", "ERROR! PINs initialization failed()");
	}
}
