/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   test02.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This test use a GPIO to make a LED blinking.
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
	// PIN configuration
	gpio_config_t io_conf;
	io_conf.intr_type    = GPIO_INTR_DISABLE;     // No interrupt
	io_conf.mode         = GPIO_MODE_OUTPUT;      // The pin is an output
	io_conf.pin_bit_mask = (1ULL << o_KEEPALIVE); // Pin number
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // NO pull-down
	io_conf.pull_up_en   = GPIO_PULLDOWN_DISABLE; // NO pull-down
	gpio_config(&io_conf);

	 while (1) {
		// Turn ON the LED
		gpio_set_level(o_KEEPALIVE, 1);
		ESP_LOGI(__FUNCTION__, "(GPIO-%d) ON", o_KEEPALIVE);
		vTaskDelay(500 / portTICK_PERIOD_MS);

		// Tutn OFF LED
		gpio_set_level(o_KEEPALIVE, 0);
		ESP_LOGI(__FUNCTION__, "OFF");
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	return;
}

