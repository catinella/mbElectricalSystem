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
#define MOCK  1

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <mbesMock.h>
#include <debugTools.h>
#include <mbesSerialConsole.h>
#include <mbesUtilities.h>
#include <mbesSelector.h>


bool loop = true;

void sigHandler (int signum) {
	printf("[!] I received a sig-%d\n", signum);
	loop = false;
	return;
}

int main (int argc, char **argv) {
	uint8_t      t = 0;
	struct       mbesSelector sel[6];
	char         *pinsList[6]  = {"A1", "A2", "B2", "B3", "D3", "D4"};
	selectorType typesList[6] = {BUTTON, BUTTON, HOLDBUTTON, HOLDBUTTON, SWITCH, SWITCH};

	// Syslog service enabling...
	MYOPENLOG

	{
		printf("\nPlease, execute the following command in another shell/terminal, and press ENTER to continue...\n");
		printf("./virtualSelectors --file=%s ", MBES_VIRTUALSEVECTOR_SWAPFILE);
		for (t=0; t<6; t++) printf("--pin=%s:%s ", pinsList[t], typesList[t] == SWITCH ? "switch" : "button");
		printf("\n");
		getchar();
	}

	// Iniyializations...
	for (t=0; t<6; t++) mbesSelector_init(&sel[t], typesList[t], pinsList[t]);

	signal(SIGTERM, sigHandler);
	signal(SIGINT,  sigHandler);

	MYSYSLOG (LOG_INFO, "========= %s =========", argv[0]);
	mbesUtilities_init();

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

			printf(" %d  (FSM=%d)\n", mbesSelector_get(sel[t]) ? 0 : 1, sel[t].fsm);
		}

		usleep(10000);
	}

	// Just to get the Valgrind-GOD's blessing
	mbesSelector_shutdown();
	USART_close();
	MYCLOSELOG

	return(0);
}
