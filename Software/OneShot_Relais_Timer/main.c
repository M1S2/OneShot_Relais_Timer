/*
 * OneShot_Relais_Timer.c
 *
 * Created: 09.10.2022 17:48:37
 * Author : Markus
 */ 

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

// ################################################################################

//#define PIN_BTN_SET			PINA1
//#define PIN_BTN_START_STOP	PINA2
#define PIN_CTRL_RELAY		PORTA4
#define PIN_BCD_7SEG_DOT	PORTA7

/*#define PIN_BCD_7SEG_1		PORTB0
#define PIN_BCD_7SEG_2		PORTB1
#define PIN_BCD_7SEG_3		PORTB2
#define PIN_BCD_7SEG_4		PORTB3*/

#define CTRL_RELAY_ON		PORTA |= (1<<PIN_CTRL_RELAY);
#define CTRL_RELAY_OFF		PORTA &= ~(1<<PIN_CTRL_RELAY);

#define SET_7SEG(number)	PORTB = number;	
#define DOT_7SEG_ON			PORTA |= (1<<PIN_BCD_7SEG_DOT);
#define DOT_7SEG_OFF		PORTA &= ~(1<<PIN_BCD_7SEG_DOT);
#define DOT_7SEG_TOGGLE		PORTA ^= (1<<PIN_BCD_7SEG_DOT);

#define IS_BTN_SET_PRESSED			(!(PINA & (1<<PA1)))
#define IS_BTN_START_STOP_PRESSED	(!(PINA & (1<<PA2)))

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

// 7-Segment-Dot not working!!!

int main(void)
{
	// Init IO registers
	DDRA &= ~(1<<DDA1);					// Set BTN_SET Pin (PA1) as input
	DDRA &= ~(1<<DDA2);					// Set BTN_START_STOP Pin (PA2) as input
	DDRA |= (1<<DDA4);					// Set CTRL_RELAY Pin (PA4) as output
	DDRA |= (1<<DDA7);					// Set BCD_7SEG_DOT Pin (PA7) as output
	DDRB = 0xF;							// Set BCD_7SEG_1 to BCD_7SEG_4 Pins (PB0..PB4) as outputs
	PORTA |= (1<<PORTA1);				// Enable Pull-Up for BTN_SET Pin (PA1)
	PORTA |= (1<<PORTA2);				// Enable Pull-Up for BTN_START_STOP Pin (PA2)
	
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
				break;
			}
		}
		
		_delay_ms(1000);
	}
}
