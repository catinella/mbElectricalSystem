//-------------------------------------------------------------------------------------------------------------------------------
// File:   mbElectricalSystem.c
//
// Author: Silvano Catinella
//
//
//	Symbol description:
//		+---------------+------------------+-----------------------------------------------------+
//		|     Symbol    |       Plug       |      Description                                    |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_VX1         | resistors-key    | first voltage reference                             |
//		| i_VX2         |                  | second  "         "                                 |
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
//-------------------------------------------------------------------------------------------------------------------------------

// DDRA |= (1 << DDA1); // Imposta il pin PA1 come uscita
// DDRA &= ~(1 << DDA1); // Imposta il pin PA1 come ingresso

// Otional:
//  PORTA |= (1 << PA1); // Abilita la resistenza di pull-up
//  PORTA |= (0 << PA1); // Abilita la resistenza di pull-down

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



void main(void) {
	
	//
	// Data Direction Registers (DDR) setting
	//
	// Port A
	DDRA &= ~(1 << DD_i_NEUTRAL);
	// Port B
	DDRB &= ~(1 << DD_i_LEFTARROW);
	DDRB &= ~(1 << DD_i_DOWNLIGHT);
	DDRB &= ~(1 << DD_i_UPLIGHT);
	DDRB &= ~(1 << DD_i_RIGHTARROW);
	DDRB &= ~(1 << DD_i_HORN);
	DDRB &= ~(1 << DD_i_BIKESTAND);
	DDRB |=  (1 << DD_o_ENGINEREADY);
	DDRB |=  (1 << DD_o_NEUTRAL);
	// Port C
	DDRC &= ~(1 << DD_i_BRAKESWITCH);
	DDRC &= ~(1 << DD_i_STARTBUTTON);
	DDRC &= ~(1 << DD_i_ENGINEON);
	DDRC &= ~(1 << DD_i_DECOMPRESS);
	DDRC |=  (1 << DD_o_STOPLIGHT);
	DDRC &= ~(1 << DD_i_ADDLIGHT);
	DDRC &= ~(1 << DD_i_LIGHTONOFF);
	// Port D
	DDRD |=  (1 << DD_o_RIGHTARROW);
	DDRD |=  (1 << DD_o_LEFTARROW);
	DDRD |=  (1 << DD_o_DOWNLIGHT);
	DDRD |=  (1 << DD_o_UPLIGHT);
	DDRD |=  (1 << DD_o_ADDLIGHT);
	DDRD |=  (1 << DD_o_HORN);
	DDRD |=  (1 << DD_o_KEEPALIVE);
	DDRD |=  (1 << DD_o_STARTENGINE);
	
	
	//
	// Pull-up/down resistors setting
	//
	PORTA |= (1 << P_i_NEUTRAL);
	// Port B
	PORTB |= (1 << P_i_LEFTARROW);
	PORTB |= (1 << P_i_DOWNLIGHT);
	PORTB |= (1 << P_i_UPLIGHT);
	PORTB |= (1 << P_i_RIGHTARROW);
	PORTB |= (1 << P_i_HORN);
	PORTB |= (1 << P_i_BIKESTAND);
	// Port C
	PORTC |= (1 << P_i_BRAKESWITCH);
	PORTC |= (1 << P_i_STARTBUTTON);
	PORTC |= (1 << P_i_ENGINEON);
	PORTC |= (1 << P_i_DECOMPRESS);
	PORTC |= (1 << P_i_ADDLIGHT);
	PORTC |= (1 << P_i_LIGHTONOFF);
	
	
	// Starting conditions
    
    return;
}
