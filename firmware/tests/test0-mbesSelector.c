/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   test0-mbesSelector.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This is the interactive simplest test for the mbesSelector module.
//	To execute the test, correctly, you have to run in another terminal (or shell) the command printed on display after the
//	test file execution. This command will open a virtual cockpit with switch and buttons, use it to verify the correct
//	selectors management performed by the mbesSelector module
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
#define MOCK 1

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <mbesMock.h>
#include <mbesSelector.h>

bool loop = true;

void sigHandler (int signum) {
	printf("[!] I received a sig-%d\n", signum);
	loop = false;
	return;
}

int main() {
	uint8_t t = 0;
	struct mbesSelector sel[6];

	mbesSelector_init(&sel[0], BUTTON,      "A1");
	mbesSelector_init(&sel[1], BUTTON,      "A2");
	mbesSelector_init(&sel[2], HOLDBUTTON,  "B2");
	mbesSelector_init(&sel[3], HOLDBUTTON,  "B3");
	mbesSelector_init(&sel[4], SWITCH,      "D3");
	mbesSelector_init(&sel[5], SWITCH,      "D4");

	signal(SIGTERM, sigHandler);
	signal(SIGINT,  sigHandler);

	while (loop) {
		for (t=0; t<6; t++) 
			mbesSelector_update (&sel[t]);

		// Display cleaning...
		printf("\e[1;1H\e[2J");

		for (t=0; t<6; t++) {
			if (sel[t].devType == BUTTON)
				printf("(%s)   ", sel[t].pin);

			else if (sel[t].devType == SWITCH)
				printf("[%s]   ", sel[t].pin);

			else if (sel[t].devType == HOLDBUTTON)
				printf("((%s)) ", sel[t].pin);

			printf(" %d\n", mbesSelector_get(sel[t]) ? 0 : 1);
		}

		usleep(10000);
	}

	return(0);
}
