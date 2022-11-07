/*
 * OneShot_Relais_Timer.c
 *
 * Created: 09.10.2022 17:48:37
 * Author : Markus
 */ 

#define F_CPU 3333333UL		// 20 MHz / 6
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "Buttons_Debounce.h"

// ################################################################################

#define PIN_CTRL_RELAY		PIN4_bp									/**< Pin used to control the relay and the led showing the relay state */
#define PIN_BCD_7SEG_DOT	PIN7_bp									/**< Pin used to control the 7-segment dot */

#define CTRL_RELAY_ON		PORTA_OUT |= (1<<PIN_CTRL_RELAY);		/**< Switch the relay on */
#define CTRL_RELAY_OFF		PORTA_OUT &= ~(1<<PIN_CTRL_RELAY);		/**< Switch the relay off */
#define CTRL_RELAY_TOGGLE	PORTA_OUT ^= (1<<PIN_CTRL_RELAY);		/**< Toggle the relay state */

#define SET_7SEG(number)	PORTB_OUT = number;						/**< Set the number displayed by the 7-segment-display */
#define DOT_7SEG_ON			PORTA_OUT |= (1<<PIN_BCD_7SEG_DOT);		/**< Switch the 7-segment dot on */
#define DOT_7SEG_OFF		PORTA_OUT &= ~(1<<PIN_BCD_7SEG_DOT);	/**< Switch the 7-segment dot off */
#define DOT_7SEG_TOGGLE		PORTA_OUT ^= (1<<PIN_BCD_7SEG_DOT);		/**< Toggle the 7-segment dot */
		
#define SECONDS_FOR_7SEG_DIGIT	(3600 * TIME_CALIBRATION_FACTOR)	/**< This number of seconds must elapse for the 7 segment digit to change */
#define TIME_CALIBRATION_FACTOR	1.02		// 0.981				/**< Factor that can be used for calibration. 1 = No calib, >1 = countdown was running to fast (slow it down), <1 = countdown was running to slow (speed it up) */

// ################################################################################

/**
 * Enumeration with the statemachine states.
 */
typedef enum States
{
	STATE_SET,							/**< SET state. This is used to modify the hours for the countdown */
	STATE_COUNTDOWN						/**< COUNTDOWN state. This is used to keep the relay on for some time */
}States_t;

volatile States_t currentState;			/**< Variable holding the current statemachine state */
uint8_t setHours;						/**< Variable holding the set hours */
volatile uint16_t remainingSeconds;		/**< Variable used to track the remaining seconds */


EEMEM uint8_t ee_defaultHours;			/**< default number of hours that are read from / saved to EEPROM */

// ################################################################################

/************************************************************************/
/* ISR for the TCA0 Overflow											*/
/* This timer is used for button handling								*/
/************************************************************************/
ISR(TCA0_OVF_vect)
{		
	debounce_timer_interrupt();	
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;		//The interrupt flag has to be cleared manually (by writing a '1' to it)
}

/************************************************************************/
/* ISR for the RTC PIT Interrupt										*/
/* This interrupt is used for counting seconds							*/
/************************************************************************/
ISR(RTC_PIT_vect)
{
	if(currentState == STATE_COUNTDOWN)
	{
		DOT_7SEG_TOGGLE
		remainingSeconds--;
	}
	
	RTC.PITINTFLAGS = RTC_PI_bm;					//The interrupt flag has to be cleared manually (by writing a '1' to it)
}

// ################################################################################

int main(void)
{
	// Init IO registers
	PORTA_DIR &= ~PIN1_bm;					// Set BTN_SET Pin (PA1) as input
	PORTA_DIR &= ~PIN2_bm;					// Set BTN_START_STOP Pin (PA2) as input
	PORTA_DIR |= PIN4_bm;					// Set CTRL_RELAY Pin (PA4) as output
	PORTA_DIR |= PIN7_bm;					// Set BCD_7SEG_DOT Pin (PA7) as output
	PORTB_DIR = 0xF;						// Set BCD_7SEG_1 to BCD_7SEG_4 Pins (PB0..PB4) as outputs
	PORTA_PIN1CTRL |= PORT_PULLUPEN_bm;		// Enable Pull-Up for BTN_SET Pin (PA1)
	PORTA_PIN2CTRL |= PORT_PULLUPEN_bm;		// Enable Pull-Up for BTN_START_STOP Pin (PA2)

	// Init Timer (TCA0, used for button handling)
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL2_bm | TCA_SINGLE_CLKSEL1_bm | TCA_SINGLE_CLKSEL0_bm | TCA_SINGLE_ENABLE_bm;	// Set Prescaler to 1024 (CLKSEL=7 (Bits 3..1)), Enable Timer (Bit 0)
	TCA0.SINGLE.PER = (uint16_t)(F_CPU / 1024 * 10e-3 + 0.5);															// preload for 10 ms
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;																			// Enable overflow interrupt
	
	// Init PIT (Periodic interrupt timer, used for counting seconds)
	RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;						// Use internal 1.024 kHz clock
	RTC.PITINTCTRL = RTC_PI_bm;								// Enable PIT interrupt
	RTC.PITCTRLA = RTC_PERIOD_CYC1024_gc | RTC_PITEN_bm;	// Set period to 1024 clock cycles (every second) and enable the PIT

	sei();
	
	currentState = STATE_SET;
	setHours = eeprom_read_byte(&ee_defaultHours);			// read the default hours from the EEPROM
	if(setHours < 1 || setHours > 9)						// read value is out of valid range. Set it to some default value.
	{
		setHours = 3;
	}
	remainingSeconds = 0;
	SET_7SEG(setHours)
	
	DOT_7SEG_OFF
	CTRL_RELAY_OFF
	
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
					SET_7SEG(setHours)
				}
				else if(get_key_press(1 << KEY_START_STOP))
				{
					// If hours are set to 0, the relay can be toggled using the start stop button. The countdown can be changed from active during countdown to non-active during countdown.
					if(setHours == 0)
					{
						CTRL_RELAY_TOGGLE
					}
					else
					{
						eeprom_update_byte(&ee_defaultHours, setHours);		// write the setHours to the EEPROM as new default value
						currentState = STATE_COUNTDOWN;
						CTRL_RELAY_TOGGLE
					}
					
					remainingSeconds = setHours * SECONDS_FOR_7SEG_DIGIT;
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
					CTRL_RELAY_TOGGLE
					setHours = eeprom_read_byte(&ee_defaultHours);		// read the default hours from the EEPROM
					SET_7SEG(setHours)
					DOT_7SEG_OFF
				}
				else
				{
					SET_7SEG(remainingSeconds / SECONDS_FOR_7SEG_DIGIT)
				}
				
				get_key_press(1 << KEY_SET);		// This is only consumed here to not trigger the set button in SET mode after leaving COUNTDOWN mode (when button was pressed in COUNTDOWN mode)
				
				break;
			}
		}		
	}
}
