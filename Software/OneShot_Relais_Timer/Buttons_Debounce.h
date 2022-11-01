/*
 * Buttons_Debounce.h
 *
 * Created: 29.10.2022 18:41:54
 *  Author: Markus
 */ 


#ifndef BUTTONS_DEBOUNCE_H_
#define BUTTONS_DEBOUNCE_H_

// !!! Key Pins must already be set as inputs and Pull-Ups must be enabled !!!
#define KEY_PIN         PORTA_IN
#define KEY_SET			1	// PINA1
#define KEY_START_STOP	2	// PINA2

#define ALL_KEYS        (1<<KEY_SET | 1<<KEY_START_STOP)

#define REPEAT_MASK     (1<<KEY_SET)		// repeat: KEY_SET
#define REPEAT_START    50					// after 500ms
#define REPEAT_NEXT     50					// every 500ms

void debounce_timer_interrupt();
uint8_t get_key_press( uint8_t key_mask );
uint8_t get_key_rpt( uint8_t key_mask );
uint8_t get_key_state( uint8_t key_mask );
uint8_t get_key_short( uint8_t key_mask );
uint8_t get_key_long( uint8_t key_mask );

#endif /* BUTTONS_DEBOUNCE_H_ */