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

//
// PINs declaration
//
#define _i_VX1          A1
#define _i_VX2          A2
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
#define _o_STOPLIGHT    C5
#define _i_ADDLIGHT     C6
#define _i_LIGHTONOFF   C7

#define _o_RIGHTARROW   D1
#define _o_LEFTARROW    D2
#define _o_DOWNLIGHT    D3
#define _o_UPLIGHT      D4
#define _o_ADDLIGHT     D5
#define _o_HORN         D6
#define _o_KEEPALIVE    D7
#define _o_STARTENGINE  D8



#define V_TOLERANCE     100



uint8_t loop = 1;
//-------------------------------------------------------------------------------------------------------------------------------
//                                                      M A I N
//-------------------------------------------------------------------------------------------------------------------------------

void main(void) {
	
	//
	// Data Direction Registers (DDR) setting
	//	TRISBbits.TRIS<PIN> = 0;  // PIN is set as output
	//	TRISBbits.TRIS<PIN> = 1;  // PIN is set as input
	//
	// Port A
	TRISBbits.TRIS_i_NEUTRAL = 1;
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
	CNPUBbits.CNPU_i_ADDLIGHT    = 1;
	CNPUBbits.CNPU_i_LIGHTONOFF  = 1;
	
	
	// Starting conditions
	LATBbits.LAT
	LATBbits.LAT_o_RIGHTARROW  = 0;
	LATBbits.LAT_o_LEFTARROW   = 0;
	LATBbits.LAT_o_DOWNLIGHT   = 0;
	LATBbits.LAT_o_UPLIGHT     = 0;
	LATBbits.LAT_o_ADDLIGHT    = 0;
	LATBbits.LAT_o_HORN        = 0;
	LATBbits.LAT_o_KEEPALIVE   = 0;  // [!] IMPORTANT!
	LATBbits.LAT_o_STARTENGINE = 0;

	while (loop) {
		//
		// Resistor keys evaluation
		//



	}

	return;
}
