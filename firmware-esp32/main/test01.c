/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   test01.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	It is the simplest provided test. It implements a simple counter and it has been written to check for the building
//	system, mainly. If you can see a conter in the serial console (idf.py monitoring), then your building chain has been
//	prperly configured.
//
//	[!] If you enable DEBUG symbol, the process will print also the configuration symbols' values. It can be useful to
//	    test the configUtils.cmake library. CMake language is not very stable and its scripts need to be tested, always!!!
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

#ifndef DEBUG
#define DEBUG 0
#endif

void app_main(void) {
	uint8_t t = 0;
	while (1) {
		ESP_LOGI(__FUNCTION__, "%d", t);
		t++;
		vTaskDelay(100 / portTICK_PERIOD_MS);
		if (t == 64) {
			vTaskDelay(500 / portTICK_PERIOD_MS);
			ESP_LOGW(__FUNCTION__, "---------------------");
			t = 0;
#if DEBUG > 0
			ESP_LOGW("DEBUG", "%s = %d", "DEBUG",      DEBUG);
			ESP_LOGW("DEBUG", "%s = %d", "NOAUTH",     NOAUTH);
			ESP_LOGW("DEBUG", "%s = %d", "NODLSWITCH", NODLSWITCH);
			ESP_LOGW(__FUNCTION__, "---------------------");
#endif
		}
	}
}
