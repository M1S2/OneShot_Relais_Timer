/*
 * OneShot_Relais_Timer.c
 *
 * Created: 09.10.2022 17:48:37
 * Author : Markus
 */ 

#define F_CPU 3300000UL		// 20 MHz / 6
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "Buttons_Debounce.h"

// ################################################################################

#define PIN_CTRL_RELAY		PIN4_bp
#define PIN_BCD_7SEG_DOT	PIN7_bp

#define CTRL_RELAY_ON		PORTA_OUT |= (1<<PIN_CTRL_RELAY);
#define CTRL_RELAY_OFF		PORTA_OUT &= ~(1<<PIN_CTRL_RELAY);

#define SET_7SEG(number)	PORTB_OUT = number;	
#define DOT_7SEG_ON			PORTA_OUT |= (1<<PIN_BCD_7SEG_DOT);
#define DOT_7SEG_OFF		PORTA_OUT &= ~(1<<PIN_BCD_7SEG_DOT);
#define DOT_7SEG_TOGGLE		PORTA_OUT ^= (1<<PIN_BCD_7SEG_DOT);

#define IS_BTN_SET_PRESSED			(!(PORTA_IN & PIN1_bm))
#define IS_BTN_START_STOP_PRESSED	(!(PORTA_IN & PIN2_bm))
		
#define SECONDS_FOR_7SEG_DIGIT	3600		// This number of seconds must elapse for the 7 segment digit to change

// ################################################################################

typedef enum States
{
	STATE_SET,
	STATE_COUNTDOWN
}States_t;

volatile States_t currentState;

uint8_t setHours;
volatile uint16_t remainingSeconds;


EEMEM uint8_t ee_defaultHours;			// default number of hours that are read from / saved to EEPROM

// ################################################################################

/*************************************************
* ISR for the TCA0 Overflow
* This timer is used for button handling
**************************************************/
ISR(TCA0_OVF_vect)
{		
	debounce_timer_interrupt();	
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;		//The interrupt flag has to be cleared manually (by writing a '1' to it)
}

/*************************************************
* ISR for the TCB0 Overflow
* This timer is used for counting seconds
**************************************************/
ISR(TCB0_INT_vect)
{
	if(currentState == STATE_COUNTDOWN)
	{
		DOT_7SEG_TOGGLE
		remainingSeconds--;
	}
	
	TCB0.INTFLAGS = TCB_CAPT_bm;					//The interrupt flag has to be cleared manually (by writing a '1' to it)
}

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

	// Init Timer (TCA0, used for button handling)
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL2_bm | TCA_SINGLE_CLKSEL1_bm | TCA_SINGLE_CLKSEL0_bm | TCA_SINGLE_ENABLE_bm;	// Set Prescaler to 1024 (CLKSEL=7 (Bits 3..1)), Enable Timer (Bit 0)
	TCA0.SINGLE.PER = (uint16_t)(F_CPU / 1024 * 10e-3 + 0.5);					// preload for 10ms
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;									// Enable overflow interrupt
	
	// Init Timer (TCB0, used for counting seconds)
	TCB0.CTRLA = TCB_CLKSEL1_bm | TCB_ENABLE_bm;								// Use clock source from TCA0, Enable Timer (Bit 0)
	TCB0.CCMP = (uint16_t)(F_CPU / 1024 * 1000e-3 + 0.5);						// preload for 1000ms
	TCB0.INTCTRL = TCB_CAPT_bm;													// Enable overflow interrupt

	sei();
	
	currentState = STATE_SET;
	setHours = eeprom_read_byte(&ee_defaultHours);	// read the default hours from the EEPROM
	if(setHours < 1 || setHours > 9)				// read value is out of valid range
	{
		setHours = 3;
	}
	remainingSeconds = 0;
	SET_7SEG(setHours)
	
	while (1)
	{		
		switch(currentState)
		{
			case STATE_SET:
			{
				if(get_key_press(1 << KEY_SET) || get_key_rpt(1 << KEY_SET))
				{
					setHours++;
					setHours = setHours % 10;
					if(setHours == 0) { setHours = 1; }		// 0 is not allowed as value
					SET_7SEG(setHours)
				}
				else if(get_key_press(1 << KEY_START_STOP))
				{
					eeprom_update_byte(&ee_defaultHours, setHours);		// write the setHours to the EEPROM as new default value
					remainingSeconds = setHours * SECONDS_FOR_7SEG_DIGIT;
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
				if(get_key_press(1 << KEY_START_STOP) || remainingSeconds <= 0)
				{
					currentState = STATE_SET;
					CTRL_RELAY_OFF
					setHours = eeprom_read_byte(&ee_defaultHours);		// read the default hours from the EEPROM
					SET_7SEG(setHours)
					DOT_7SEG_OFF
				}
				else
				{
					SET_7SEG(remainingSeconds / SECONDS_FOR_7SEG_DIGIT)
				}
				break;
			}
		}		
	}
}
