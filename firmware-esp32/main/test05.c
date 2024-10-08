/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   test05.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	It makes the led blinking using an ESP32's timer. Use the button to ends the test and release the used resources
//
//	5V --------------------+-----+------------------------------------------
//	                       |     |
//	                     +-+-+   |  +-------+
//	                     |10K|   +--+       |        +---------+
//	                     |OHM|      |       |        |         |
//	                     +-+-+      |       +------->|   USB   |
//	              _--_     |        |       |        | Console |
//	          +---O  O-----+--------+  MCU  |        |         |
//	          |  (STOP)             |       |        +---------+
//	          |                     |       |
//	          |                     |       |              +--------+
//	          |                  +--+       +-------[LED]--+ 2K Ohm +---+
//	          |                  |  +-------+              +--------+   |
//	          |                  |                                      |
//	GND ------+------------------+--------------------------------------+---
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


void ledStatus(void *status) {
	static bool ist = false;
	static SemaphoreHandle_t ledMutex;
	static bool flag = false;
	
	if (flag == false) {
		ledMutex = xSemaphoreCreateMutex();
		flag = true;
	}
	
	if (xSemaphoreTake(ledMutex, portMAX_DELAY) == pdTRUE) {
		if (status == NULL) {
			ist = ist ? false : true;
		} else {
			*(bool*)status = ist;
		}
		xSemaphoreGive(ledMutex);
	}
	
	return;
}
	
void app_main(void) {
	esp_timer_create_args_t timerArgs;
	esp_timer_handle_t      timerHandle;
	void                    *nAddr = NULL;
	gpio_config_t           ledGpio, stopBtn;


	//
	// PIN configuration
	//
	ledGpio.intr_type    = GPIO_INTR_DISABLE;
	ledGpio.mode         = GPIO_MODE_OUTPUT;
	ledGpio.pin_bit_mask = (1ULL << o_KEEPALIVE);
	ledGpio.pull_down_en = GPIO_PULLDOWN_DISABLE;
	ledGpio.pull_up_en   = GPIO_PULLDOWN_DISABLE;

	
	//
	// PIN configuration
	//
	stopBtn.intr_type    = GPIO_INTR_DISABLE;
	stopBtn.mode         = GPIO_MODE_INPUT;
	stopBtn.pin_bit_mask = (1ULL << i_CONF1);
	stopBtn.pull_down_en = GPIO_PULLDOWN_DISABLE;
	stopBtn.pull_up_en   = GPIO_PULLDOWN_DISABLE;

	
	//
	// Timer configuration
	//
	timerArgs.callback              = ledStatus;
	timerArgs.arg                   = nAddr;
	timerArgs.dispatch_method       = ESP_TIMER_TASK;
	timerArgs.name                  = "led blinking timer";
	timerArgs.skip_unhandled_events = true;

	
	if (gpio_config(&ledGpio) != ESP_OK || gpio_config(&stopBtn) != ESP_OK)
		// ERROR!
		ESP_LOGE("-->", "ERROR! gpio_config() failed");
	
	else if (esp_timer_create(&timerArgs, &timerHandle) != ESP_OK) 
		// ERROR!
		ESP_LOGE("-->", "ERROR! I cannot crate the timer");
	
	else if (esp_timer_start_periodic(timerHandle, 150000) != ESP_OK) 
		// ERROR!
		ESP_LOGE("-->", "ERROR! I cannot start the timer");
		
	else {
		bool status = false, loop = true;
		while (loop) {
			if (gpio_get_level(i_CONF1) == 1) {
				ledStatus((void*)&status);
				gpio_set_level(o_KEEPALIVE, status ? 1 : 0);
			} else
				// Exit
				loop = false;
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
		if (esp_timer_stop(timerHandle) != ESP_OK)
			// ERROR!
			ESP_LOGE("-->", "ERROR! I cannot stop the timer");
		else
			ESP_LOGI("-->", "Times has been stopped successfully");
		
		if (esp_timer_delete(timerHandle) != ESP_OK)
			// ERROR!
			ESP_LOGE("-->", "ERROR! I cannot delete the timer");
		else
			ESP_LOGI("-->", "Times has been released successfully");
			
		// Turning off the led	
		gpio_set_level(o_KEEPALIVE, 0);
	}
}
