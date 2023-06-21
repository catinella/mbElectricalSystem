/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
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
------------------------------------------------------------------------------------------------------------------------------*/


#include <avr/io.h>
#include <stdlib.h>


//
// PINs declaration
//
#define i_VX1           0
#define i_VX2           1
#define i_VY1           2
#define i_VY2           3
// === availablre ===  {'A', 4}
// === availablre ===  {'A', 5}
#define o_ENGINEOFF    {'A', 6}
#define i_NEUTRAL      {'A', 7}

#define i_LEFTARROW    {'B', 0}
#define i_DOWNLIGHT    {'B', 1}
#define i_UPLIGHT      {'B', 2}
#define i_RIGHTARROW   {'B', 3}
#define i_HORN         {'B', 4}
#define i_BIKESTAND    {'B', 5}
#define o_ENGINEREADY  {'B', 6}
#define o_NEUTRAL      {'B', 7}

#define i_BRAKESWITCH  {'C', 0}
#define i_STARTBUTTON  {'C', 1}
#define i_ENGINEON     {'C', 2}
#define i_DECOMPRESS   {'C', 3}
#define i_POWEROFF     {'C', 4}
#define o_STOPLIGHT    {'C', 5}
#define i_ADDLIGHT     {'C', 6}
#define i_LIGHTONOFF   {'C', 7}

#define o_RIGHTARROW   {'D', 0}
#define o_LEFTARROW    {'D', 1}
#define o_DOWNLIGHT    {'D', 2}
#define o_UPLIGHT      {'D', 3}
#define o_ADDLIGHT     {'D', 4}
#define o_HORN         {'D', 5}
#define o_KEEPALIVE    {'D', 6}
#define o_STARTENGINE  {'D', 7}


#define BLINK_DELAY     4000000
#define V_TOLERANCE     100
#define BUTTON_DEBOUNC  10000

struct avrPin {
	char port;
	uint8_t number;
};

typedef enum _mbesPinDir {
	INPUT,
	OUTPUT
} mbesPinDir;

//------------------------------------------------------------------------------------------------------------------------------
//                                                 F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

uint16_t ADC_read(uint8_t channel) {
	ADMUX &= 0x0F;                  // Cancella i bit precedenti del canale
	ADMUX |= channel;              // Analog channel selection
	
	ADCSRA |= (1 << ADSC);         // Convertion starting...

	while (ADCSRA & (1 << ADSC));  // Waiting for convertion operation

	return ADC;
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


void pinDirectionRegister (struct avrPin pin, mbesPinDir dir) {
	//
	// Description:
	//	Data Direction Registers (DDR) setting
	//

	if      (pin.port == 'A' && dir == OUTPUT)  DDRA |=  (1 << pin.number);
	else if (pin.port == 'A')                   DDRA &= ~(1 << pin.number);
	else if (pin.port == 'B' && dir == OUTPUT)  DDRB |=  (1 << pin.number);
	else if (pin.port == 'B')                   DDRB &= ~(1 << pin.number);
	else if (pin.port == 'C' && dir == OUTPUT)  DDRC |=  (1 << pin.number);
	else if (pin.port == 'C')                   DDRC &= ~(1 << pin.number);
	else if (pin.port == 'D' && dir == OUTPUT)  DDRD |=  (1 << pin.number);
	else if (pin.port == 'D')                   DDRD &= ~(1 << pin.number);
	else {
		// ERROR!
	}

	return;
}


void pullUpEnabling (struct avrPin pin) {
	//
	// Description
	//	It enable the pull-up resistor for the argument defined input pin
	//

	if      (pin.port == 'A') PORTA |= (1 << pin.number);
	else if (pin.port == 'B') PORTB |= (1 << pin.number);
	else if (pin.port == 'C') PORTC |= (1 << pin.number);
	else if (pin.port == 'D') PORTD |= (1 << pin.number);
	else {
		// ERROR!
	}

	return;
}


uint8_t getPinValue (struct avrPin pin) {
	//
	// Description
	//	It returns the input pin's current value
	//
	uint8_t out = 0;

	if      (pin.port == 'A') out = (PINA & (1 << pin.number));
	else if (pin.port == 'B') out = (PINB & (1 << pin.number));
	else if (pin.port == 'C') out = (PINC & (1 << pin.number));
	else if (pin.port == 'D') out = (PIND & (1 << pin.number));
	else {
		// ERROR!
	}

	return((out > 0) ? out : 1);
}


void setPinValue (struct avrPin pin, uint8_t value) {
	//
	// Description
	//	It returns the input pin's current value
	//

	if      (pin.port == 'A' && value) PORTA |=  (1 << pin.number);
	if      (pin.port == 'A')          PORTA &= ~(1 << pin.number);
	else if (pin.port == 'B' && value) PORTB |=  (1 << pin.number);
	else if (pin.port == 'B')          PORTB &= ~(1 << pin.number);
	else if (pin.port == 'C' && value) PORTC |=  (1 << pin.number);
	else if (pin.port == 'C')          PORTC &= ~(1 << pin.number);
	else if (pin.port == 'D' && value) PORTD |=  (1 << pin.number);
	else if (pin.port == 'D')          PORTD &= ~(1 << pin.number);
	else {
		// ERROR!
	}

	return((out > 0) ? out : 1);
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
	// PINs direction setting
	//

	ANSELbits.ANS_i_VX1          = 1;
	ANSELbits.ANS_i_VX2          = 1;
	ANSELbits.ANS_i_VY1          = 1;
	ANSELbits.ANS_i_VY2          = 1;

	pinDirectionRegister(i_NEUTRAL,     INPUT);
	pinDirectionRegister(i_LEFTARROW,   INPUT);
	pinDirectionRegister(i_DOWNLIGHT,   INPUT);
	pinDirectionRegister(i_UPLIGHT,     INPUT);
	pinDirectionRegister(i_RIGHTARROW,  INPUT);
	pinDirectionRegister(i_HORN,        INPUT);
	pinDirectionRegister(i_BIKESTAND,   INPUT);
	pinDirectionRegister(i_BRAKESWITCH, INPUT);
	pinDirectionRegister(i_STARTBUTTON, INPUT);
	pinDirectionRegister(i_ENGINEON,    INPUT);
	pinDirectionRegister(i_DECOMPRESS,  INPUT);
	pinDirectionRegister(i_POWEROFF,    INPUT);
	pinDirectionRegister(i_ADDLIGHT,    INPUT);
	pinDirectionRegister(i_LIGHTONOFF,  INPUT);

	pinDirectionRegister(o_ENGINEOFF,   OUTPUT);
	pinDirectionRegister(o_ENGINEREADY, OUTPUT);
	pinDirectionRegister(o_NEUTRAL,     OUTPUT);
	pinDirectionRegister(o_STOPLIGHT,   OUTPUT);
	pinDirectionRegister(o_RIGHTARROW,  OUTPUT);
	pinDirectionRegister(o_LEFTARROW,   OUTPUT);
	pinDirectionRegister(o_DOWNLIGHT,   OUTPUT);
	pinDirectionRegister(o_UPLIGHT,     OUTPUT);
	pinDirectionRegister(o_ADDLIGHT,    OUTPUT);
	pinDirectionRegister(o_HORN,        OUTPUT);
	pinDirectionRegister(o_KEEPALIVE,   OUTPUT);
	pinDirectionRegister(o_STARTENGINE, OUTPUT);
	
	
	//
	// Pull-up/down resistors setting
	//

	pullUpEnabling(i_NEUTRAL);
	pullUpEnabling(i_LEFTARROW);
	pullUpEnabling(i_DOWNLIGHT);
	pullUpEnabling(i_UPLIGHT);
	pullUpEnabling(i_RIGHTARROW);
	pullUpEnabling(i_HORN);
	pullUpEnabling(i_BIKESTAND);
	pullUpEnabling(i_BRAKESWITCH);
	pullUpEnabling(i_STARTBUTTON);
	pullUpEnabling(i_ENGINEON);
	pullUpEnabling(i_DECOMPRESS);
	pullUpEnabling(i_POWEROFF);
	pullUpEnabling(i_ADDLIGHT);
	pullUpEnabling(i_LIGHTONOFF);
	
	
	//
	// A/D converter settings....
	//
	ADMUX  = (1 << REFS0);   // Voltage reference to AVCC
	ADCSRA = (1 << ADEN);    // A/D converter enabling

	
	//
	// Starting conditions
	//
	setPinValue(o_RIGHTARROW,  0);
	setPinValue(o_LEFTARROW,   0);
	setPinValue(o_DOWNLIGHT,   0);
	setPinValue(o_UPLIGHT,     0);
	setPinValue(o_ADDLIGHT,    0);
	setPinValue(o_HORN,        0);
	setPinValue(o_KEEPALIVE,   0); // IMPORTANT!!
	setPinValue(o_STARTENGINE, 0);
	setPinValue(o_ENGINEOFF,   1);



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
				setPinValue(o_KEEPALIVE, 1);
				ready_flag = 1;
			}
		
		} else if (getPinValue(i_POWEROFF) == 0) {
			//
			// Security POWER-OFF
			//
			setPinValue(o_ENGINEOFF, 1); // Stop the motorbike's engine
			setPinValue(o_KEEPALIVE, 0); // Turn off myself (paranoide solution)
			ready_flag = 0;              // If the turning off op failed (ultra paranoide)
			decompress_flag = 0;         //  ""
		
		} else {
			//
			// Lights and horn
			//
			setPinValue(o_UPLIGHT,     getPinValue(i_UPLIGHT));
			setPinValue(o_DOWNLIGHT,   getPinValue(i_DOWNLIGHT));
			setPinValue(o_HORN,        getPinValue(i_HORN));
			setPinValue(o_ADDLIGHT,    getPinValue(i_ADDLIGHT));
			//setPinValue(o_BRAKESWITCH, getPinValue(i_BRAKESWITCH));
			setPinValue(o_NEUTRAL,     getPinValue(i_NEUTRAL));


			//
			// Blinking lights
			//
			if (getPinValue(i_LEFTARROW)  == 0) {
				setPinValue(o_LEFTARROW,  blink());
				setPinValue(o_RIGHTARROW, 0);

			} else if (getPinValue(i_RIGHTARROW) == 0) {
				setPinValue(o_RIGHTARROW, blink());
				setPinValue(o_LEFTARROW,  0);

			} else {
				setPinValue(o_RIGHTARROW, 0);
				setPinValue(o_LEFTARROW,  0);
			}
			

			if (getPinValue(i_ENGINEON) == 0) {
				//
				// "_i_ENGINEON" is enablen (0) when the switch placed in the right hand-bar is set to "RUN".
				// In different case, It stop the engine and does not allow the motorbike engine to start or
				// re-start.
				//
				
				// Disabling CDI blocking
				setPinValue(o_ENGINEOFF, 0);

				// Decompressor sensor management
				if (getPinValue(i_DECOMPRESS) == 0) decompress_flag = 1;
				
				if (
					decompress_flag &&
					getPinValue(i_NEUTRAL) == 0 && 
					getPinValue(i_STARTBUTTON) == 0
				) {
					engstart_flag = 1;
					decompress_flag = 0;
					setPinValue(o_STARTENGINE, 1); // The electric starter motor is rounding!!
				}

			} else {
				decompress_flag = 0;             // it resets decompressor status
				setPinValue(o_ENGINEREADY, 0);   // it turns-off the led indicator
				setPinValue(o_ENGINEOFF,   1);   // it stops the motorbike engine using CDI. (paranoide option)
			}

			if (engstart_flag) {
				// The electric starter is running...
				if (getPinValue(i_STARTBUTTON) == 0)
					// The button is still pressed
					startButtonDebouncing = BUTTON_DEBOUNC;
				else
					// Probably the button has been released
					startButtonDebouncing--;

				if (startButtonDebouncing == 0) {
					// OK the start button has been released
					setPinValue(o_STARTENGINE, 0);
					engstart_flag = 0;
				}
			}
		}
	}

	return;
}
