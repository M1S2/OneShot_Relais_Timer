/*
 * OneShot_Relais_Timer.c
 *
 * Created: 09.10.2022 17:48:37
 * Author : Markus
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <util/delay.h>

// ################################################################################

//#define PIN_BTN_SET			PINA1
//#define PIN_BTN_START_STOP	PINA2
#define PIN_CTRL_RELAY			PIN4_bp
#define PIN_BCD_7SEG_DOT		PIN7_bp

/*#define PIN_BCD_7SEG_1		PIN0_bp
#define PIN_BCD_7SEG_2			PIN1_bp
#define PIN_BCD_7SEG_3			PIN2_bp
#define PIN_BCD_7SEG_4			PIN3_bp */

#define CTRL_RELAY_ON		PORTA_OUT |= (1<<PIN_CTRL_RELAY);
#define CTRL_RELAY_OFF		PORTA_OUT &= ~(1<<PIN_CTRL_RELAY);

#define SET_7SEG(number)	PORTB_OUT = number;	
#define DOT_7SEG_ON			PORTA_OUT |= (1<<PIN_BCD_7SEG_DOT);
#define DOT_7SEG_OFF		PORTA_OUT &= ~(1<<PIN_BCD_7SEG_DOT);
#define DOT_7SEG_TOGGLE		PORTA_OUT ^= (1<<PIN_BCD_7SEG_DOT);

#define IS_BTN_SET_PRESSED			(!(PORTA_IN & PIN1_bm))
#define IS_BTN_START_STOP_PRESSED	(!(PORTA_IN & PIN2_bm))

#define DEFAULT_HOURS		3

// ################################################################################

typedef enum States
{
	STATE_SET,
	STATE_COUNTDOWN
}States_t;

States_t currentState;

uint8_t setHours;
uint16_t actualSeconds;

// ################################################################################

int main(void)
{
	// Init IO registers
	PORTA_DIR &= ~PIN1_bm;				// Set BTN_SET Pin (PA1) as input
	PORTA_DIR &= ~PIN2_bm;				// Set BTN_START_STOP Pin (PA2) as input
	PORTA_DIR |= PIN4_bm;				// Set CTRL_RELAY Pin (PA4) as output
	PORTA_DIR |= PIN7_bm;				// Set BCD_7SEG_DOT Pin (PA7) as output
	PORTB_DIR = 0xF;					// Set BCD_7SEG_1 to BCD_7SEG_4 Pins (PB0..PB4) as outputs
	PORTA_PIN1CTRL |= PORT_PULLUPEN_bm;	// Enable Pull-Up for BTN_SET Pin (PA1)
	PORTA_PIN2CTRL |= PORT_PULLUPEN_bm;	// Enable Pull-Up for BTN_START_STOP Pin (PA2)
	
	currentState = STATE_SET;
	setHours = DEFAULT_HOURS;
	actualSeconds = 0;
	SET_7SEG(setHours)
	
	while (1)
	{		
		switch(currentState)
		{
			case STATE_SET:
			{
				if(IS_BTN_SET_PRESSED)
				{
					setHours++;
					setHours = setHours % 10;
					SET_7SEG(setHours)
				}
				else if(IS_BTN_START_STOP_PRESSED)
				{
					actualSeconds = setHours * 1;		//* 3600; !!!
					currentState = STATE_COUNTDOWN;
					CTRL_RELAY_ON
				}
				else
				{
					// Nothing to do here
				}
				break;
			}
			case STATE_COUNTDOWN:
			{
				DOT_7SEG_TOGGLE
				if(IS_BTN_START_STOP_PRESSED)
				{
					// Actions are outside of this if statement	
				}
				else
				{
					actualSeconds--;
					if(actualSeconds <= 0)
					{
						// Actions are outside of this if statement
					}
					else
					{
						SET_7SEG(actualSeconds / 1)		// / 3600) !!!
						break;
					}
				}
				
				currentState = STATE_SET;
				CTRL_RELAY_OFF
				setHours = DEFAULT_HOURS;
				SET_7SEG(setHours)
				DOT_7SEG_OFF
				break;
			}
		}
		
		_delay_ms(1000);
	}
}
