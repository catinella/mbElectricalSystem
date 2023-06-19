/*-------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
//  |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
//  | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
//  | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
//  |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                  |___/                       
//
// File:   mbElectricalSystem.c
//
// Author: Silvano Catinella
//
// Description:
//
//	Symbol description:
//		+---------------+------------------+-----------------------------------------------------+
//		|     Symbol    |    Plug/link     |      Description                                    |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_VX1         | resistors-key    | first voltage value                                 |
//		| i_VX2         |                  | second  "       "                                   |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_VY1         |  internal link   | first voltage reference                             |
//		| i_VY2         |                  | second  "         "                                 |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_NEUTRAL     | from gearbox     | it is 0 when the gear is in neutral position        |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_DECOMPRESS  | from decompress. | it is 0 when decompressor has been pushes           |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_KEEPALIVE   | internal only    | it keeps the system on when you unplug the key      |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_LEFTARROW   |                  | left turn blinking indicator switch                 |
//		| i_DOWNLIGHT   |                  | low beam command                                    |
//		| i_UPLIGHT     | left controls    | dazzling beam command                               |
//		| i_RIGHTARROW  |                  | right turn blinking indicator switch                |
//		| i_HORN        |                  | horn button                                         |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_BIKESTAND   | from bikestand   | it is 0 when the bike is placed on the stand        |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_STARTBUTTON |                  | the tipical motorbike engine start button           |
//		| i_BRAKESWITCH | right controls   | the switch on the front brake command               |
//		| i_ENGINEON    |                  | the on/off engine switch in on-position             |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_ADDLIGHT    | from ext. switch | additional high power light                         |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_LIGHTONOFF  | from ext. switch | it turns on the normal lights (low/dazzling beam)   |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_STOPLIGHT   |  rear mtb. side  | it turns on the rear STOP light                     |
//		| o_STARTENGINE |                  | it activates the (NO) engine-relay                  |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_ENGINEREADY |                  |                                                     |
//		| o_NEUTRAL     |                  |                                                     |
//		| o_RIGHTARROW  |   cockpit LED    |                                                     |
//		| o_LEFTARROW   | indicators and   |                                                     |
//		| o_DOWNLIGHT   |    services      |                                                     |
//		| o_UPLIGHT     |                  |                                                     |
//		| o_ADDLIGHT    |                  |                                                     |
//		| o_HORN        |                  |                                                     |
//		+---------------+------------------+-----------------------------------------------------+
//
-------------------------------------------------------------------------------------------------------------------------------*/


#include <xc.h>
#include <stdlib.h>


//
// PINs declaration
//
#define _i_VX1          A1
#define _i_VX2          A2
#define _i_VY1          A3
#define _i_VY2          A4
// === availablre ===   A5
// === availablre ===   A6
#define _o_ENGINEOFF    A7
#define _i_NEUTRAL      A8

#define _i_LEFTARROW    B1
#define _i_DOWNLIGHT    B2
#define _i_UPLIGHT      B3
#define _i_RIGHTARROW   B4
#define _i_HORN         B5
#define _i_BIKESTAND    B6
#define _o_ENGINEREADY  B7
#define _o_NEUTRAL      B8

#define _i_BRAKESWITCH  C1
#define _i_STARTBUTTON  C2
#define _i_ENGINEON     C3
#define _i_DECOMPRESS   C4
#define _i_POWEROFF     C5
#define _o_STOPLIGHT    C6
#define _i_ADDLIGHT     C7
#define _i_LIGHTONOFF   C8

#define _o_RIGHTARROW   D1
#define _o_LEFTARROW    D2
#define _o_DOWNLIGHT    D3
#define _o_UPLIGHT      D4
#define _o_ADDLIGHT     D5
#define _o_HORN         D6
#define _o_KEEPALIVE    D7
#define _o_STARTENGINE  D8


#define BLINK_DELAY     4000000
#define V_TOLERANCE     100
#define BUTTON_DEBOUNC  10000

uint16_t ADC_read(uint8_t channel) {
	//
	// Description:
	//	It allows you to read the voltage value of the argument defined PIN
	//
	uint16_t out = 0;

	ADCON0bits.CHS = channel;     // Seleziona il canale di ingresso analogico
	ADCON0bits.GO = 1;            // Avvia la conversione

	while (ADCON0bits.GO);        // Attendi il completamento della conversione
	out = (ADRESH << 8) | ADRESL;

	return(out);
}


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


//-------------------------------------------------------------------------------------------------------------------------------
//                                                      M A I N
//-------------------------------------------------------------------------------------------------------------------------------

void main(void) {
	uint8_t  loop                  = 1; // It enables the main loop (Just for future applications)
	uint8_t  ready_flag            = 0; // When the flag is true (1), the motorbike is ready to accept commands
	uint8_t  decompress_flag       = 0; // It is set when the decompressor lever has been pulled
	uint8_t  engstart_flag         = 1; // It means the driver is pushing the start button and the electric engine is running
	uint16_t startButtonDebouncing = 0; // It is the counter used to manage the start button bebouncing event

	//
	// Data Direction Registers (DDR) and analog setting
	//	TRISBbits.TRIS<PIN> = 0;  // PIN is set as output
	//	TRISBbits.TRIS<PIN> = 1;  // PIN is set as input
	//	ANSELbits.ANS<PIN> = 1:   // PIN is set as analog input
	//

	// Port A
	ANSELbits.ANS_i_VX1          = 1;
	ANSELbits.ANS_i_VX2          = 1;
	ANSELbits.ANS_i_VY1          = 1;
	ANSELbits.ANS_i_VY2          = 1;
	ANSELbits.ANS_o_ENGINEOFF    = 0;
	TRISBbits.TRIS_i_NEUTRAL     = 1;
	// Port B
	TRISBbits.TRIS_i_LEFTARROW   = 1;
	TRISBbits.TRIS_i_DOWNLIGHT   = 1;
	TRISBbits.TRIS_i_UPLIGHT     = 1;
	TRISBbits.TRIS_i_RIGHTARROW  = 1;
	TRISBbits.TRIS_i_HORN        = 1;
	TRISBbits.TRIS_i_BIKESTAND   = 1;
	TRISBbits.TRIS_o_ENGINEREADY = 0;
	TRISBbits.TRIS_o_NEUTRAL     = 0;
	// Port C
	TRISBbits.TRIS_i_BRAKESWITCH = 1;
	TRISBbits.TRIS_i_STARTBUTTON = 1;
	TRISBbits.TRIS_i_ENGINEON    = 1;
	TRISBbits.TRIS_i_DECOMPRESS  = 1;
	TRISBbits.TRIS_i_POWEROFF    = 1;
	TRISBbits.TRIS_o_STOPLIGHT   = 0;
	TRISBbits.TRIS_i_ADDLIGHT    = 1;
	TRISBbits.TRIS_i_LIGHTONOFF  = 1;
	// Port D
	TRISBbits.TRIS_o_RIGHTARROW  = 0;
	TRISBbits.TRIS_o_LEFTARROW   = 0;
	TRISBbits.TRIS_o_DOWNLIGHT   = 0;
	TRISBbits.TRIS_o_UPLIGHT     = 0;
	TRISBbits.TRIS_o_ADDLIGHT    = 0;
	TRISBbits.TRIS_o_HORN        = 0;
	TRISBbits.TRIS_o_KEEPALIVE   = 0;
	TRISBbits.TRIS_o_STARTENGINE = 0;
	
	
	//
	// Pull-up/down resistors setting
	//	CNPUBbits.CNPU<PIN> = 1;    // Pull-up resistors on the PIN
	//	CNPUBbits.CNPD<PIN> = 1;    // Pull-down resistors on the PIN

	CNPUBbits.CNPU
	CNPUBbits.CNPU_i_NEUTRAL     = 1;
	// Port B
	CNPUBbits.CNPU_i_LEFTARROW   = 1;
	CNPUBbits.CNPU_i_DOWNLIGHT   = 1;
	CNPUBbits.CNPU_i_UPLIGHT     = 1;
	CNPUBbits.CNPU_i_RIGHTARROW  = 1;
	CNPUBbits.CNPU_i_HORN        = 1;
	CNPUBbits.CNPU_i_BIKESTAND   = 1;
	// Port C
	CNPUBbits.CNPU_i_BRAKESWITCH = 1;
	CNPUBbits.CNPU_i_STARTBUTTON = 1;
	CNPUBbits.CNPU_i_ENGINEON    = 1;
	CNPUBbits.CNPU_i_DECOMPRESS  = 1;
	CNPUBbits.CNPU_i_POWEROFF    = 1;
	CNPUBbits.CNPU_i_ADDLIGHT    = 1;
	CNPUBbits.CNPU_i_LIGHTONOFF  = 1;
	
	
	//
	// A/D converter settings....
	//
	ADCON0bits.ADON = 1;          // ADC enabling
	ADCON1bits.ADCS = 0b111;      // Prescaler set to 128 for an adequate scan frequency
	ADCON1bits.ADFM = 1;          // Right data alingment. A/D returns 10-bit integer
	ADCON1bits.VCFG0 = 0;         // Imposta la tensione di riferimento bassa su Vss
	ADCON1bits.VCFG1 = 0;         // Imposta la tensione di riferimento alta su Vdd

	
	//
	// Starting conditions
	//
	LATBbits.LAT_o_RIGHTARROW  = 0;
	LATBbits.LAT_o_LEFTARROW   = 0;
	LATBbits.LAT_o_DOWNLIGHT   = 0;
	LATBbits.LAT_o_UPLIGHT     = 0;
	LATBbits.LAT_o_ADDLIGHT    = 0;
	LATBbits.LAT_o_HORN        = 0;
	LATBbits.LAT_o_KEEPALIVE   = 0;  // [!] IMPORTANT!
	LATBbits.LAT_o_STARTENGINE = 0;
	LATBbits.LAT_o_ENGINEOFF   = 1;



	while (loop) {
		if (ready_flag == 0) {
			//
			// Resistor keys evaluation
			//
			if (
				abs(ADC_read(_i_VX1) - ADC_read(_i_VY1)) < V_TOLERANCE &&
				abs(ADC_read(_i_VX2) - ADC_read(_i_VY2)) < V_TOLERANCE 
			) {
				// The keyword has been authenicated, you can unplug it
				LATBbits.LAT_o_KEEPALIVE = 1;
				ready_flag = 1;
			}
		
		} else if (LATBbits.LAT_i_POWEROFF == 0) {
			//
			// Security POWER-OFF
			//
			LATBbits.LAT_o_KEEPALIVE = 0;
			ready_flag = 0;
			decompress_flag = 0;
			LATBbits.LAT_o_ENGINEOFF = 1;
		
		} else {
			//
			// Lights and horn
			//
			LATBbits.LAT_o_UPLIGHT     = ! LATBbits.LAT_i_UPLIGHT;
			LATBbits.LAT_o_DOWNLIGHT   = ! LATBbits.LAT_i_DOWNLIGHT;
			LATBbits.LAT_o_HORN        = ! LATBbits.LAT_i_HORN;
			LATBbits.LAT_o_ADDLIGHT    = ! LATBbits.LAT_i_ADDLIGHT;
			LATBbits.LAT_o_BRAKESWITCH = ! LATBbits.LAT_i_BRAKESWITCH;
			LATBbits.LAT_o_NEUTRAL     = ! LATBbits.LAT_i_NEUTRAL;

			if (LATBbits.LAT_i_LEFTARROW  == 0) {
				LATBbits.LAT_o_LEFTARROW = blink();
				LATBbits.LAT_o_RIGHTARROW = 0;

			} else if (LATBbits.LAT_i_RIGHTARROW == 0) {
				LATBbits.LAT_o_RIGHTARROW = blink();
				LATBbits.LAT_o_LEFTARROW = 0;

			else {
				LATBbits.LAT_o_RIGHTARROW = 0;
				LATBbits.LAT_o_LEFTARROW = 0;
			}
			
			// Is the gearbox in neutral position?
			if (LATBbits.LAT_i_NEUTRAL == 0) neutral_flag = 1;

			if (LATBbits.LAT_i_ENGINEON == 0) {
				//
				// "_i_ENGINEON" is enablen (0) when the switch placed in the right hand-bar is set to "RUN".
				// In different case, It stop the engine and does not allow the motorbike engine to start or
				// re-start.
				//

				LATBbits.LAT_o_ENGINEOFF = 0;

				// Decompressor sensor management
				if (LATBbits.LAT_i_DECOMPRESS  == 0) decompress_flag = 1;
				
				if (decompress_flag && neutral_flag && LATBbits.LAT_i_STARTBUTTON == 0)
					engstart_flag = 1;
					decompress_flag = 0;
					LATBbits.LAT_o_STARTENGINE = 1; // START!!
				}

			} else {
				decompress_flag = 0;            // reset decompressor status
				LATBbits.LAT_o_ENGINEREADY = 0; // turn-off the led indicator
				LATBbits.LAT_o_ENGINEOFF = 1;   // STOP the motorbike engine using CDI
			}

			if (engstart_flag) {
				// The electric starter is running...
				if (LATBbits.LAT_i_STARTBUTTON == 0)
					// The button is still pressed
					startButtonDebouncing = BUTTON_DEBOUNC;
				else
					// Probably the button has been released
					startButtonDebouncing--;

				if (startButtonDebouncing == 0) {
					// OK the start button has been released
					LATBbits.LAT_o_STARTENGINE = 0;
					engstart_flag = 0;
				}
			}
		}
	}

	return;
}
