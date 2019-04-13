/*
* Name: echo.c
* Author:Group 6
* Created: 1:20:17 13-03-2018
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

const float realtime = 0.03555501302083333;

int uart_putchar(char c, FILE *stream);

int uart_getchar(FILE *stream);

FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE mystdin = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

enum machine_states {waiting, set_trigger, triggering, ranging, calculating};
enum machine_states state;

enum echo_state {low, high};
enum echo_state input_state;

unsigned short overflow, starting_cnt, ending_cnt, distance;
float time_difference;

void speaker() {
		DDRB = (1<<PB3) | (1<<PB1) ;
		TCNT0 = 0;
		TCCR0A = (1<<WGM01) | (1<<COM0A0);
		TCCR0B = (1<<CS02)  | (1<<CS00);
		if ((distance <= 36)){
			OCR0A = 27;
		}
			if ((distance >36 && distance <72)){
				OCR0A = 25;
	}
if ((distance >=72 && distance <109)){
	OCR0A = 23;
}
if ((distance >=109 && distance <145)){
	OCR0A = 21;
}
if ((distance >=145 && distance <181)){
	OCR0A = 20;
}
if ((distance >=181 && distance <218)){
	OCR0A = 19;
}
if ((distance >=218 && distance <254)){
	OCR0A = 18;
}
if ((distance >=254 && distance <290)){
OCR0A = 17;
}
if ((distance >=290 && distance <327)){
	OCR0A = 16;
	
}
if ((distance >=327 && distance <363)){
	OCR0A = 15;
}
}

int uart_putchar(char c, FILE *stream) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

int uart_getchar(FILE *stream) {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

void init_uart(void) {
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	UBRR0 = 95;
	stdout = &mystdout;
	stdin = &mystdin;
}

ISR(TIMER1_COMPA_vect) {
	if (state == triggering) {
		state = ranging;
		TCNT1 = 0;
		TCCR1B = (0x01<<ICNC1) | (0x01<<ICES1) | (0x01<<CS11);
		TIMSK1 = (0x01<<TOIE1) | (0x01<<ICIE1);
		PORTB &= ~(0x01<<PB1);
	}
}

ISR(TIMER1_CAPT_vect) {
	if((input_state == low) && (state == ranging)) {
		TCNT1 = 0;
		starting_cnt = ICR1;
		TCCR1B &= ~(0x01<<ICES1);
		input_state = high;
		} else if ((input_state == high) && (state == ranging)) {
		ending_cnt = ICR1;
		input_state = low;
		state = calculating;
	}
}

ISR(TIMER1_OVF_vect) {
	if (state == ranging) {
		++overflow;
		if (overflow >= 5) {
			overflow = 0;
			TCCR1B = 0;
			TIMSK1 = 0;
			input_state = low;
			state = set_trigger;
			TIFR1 = 0xFF;
		}
	}
}




void trigger() {
	if (state == set_trigger) {
		state = triggering;
		TIMSK1 = (0x01<<OCIE1A);
		TCCR1B = (0x01<<ICNC1) | (0x01<<ICES1) | (0x01<<WGM12) | (0x01<<CS11);
		OCR1A = 18;
		PORTB |= (0x01<<PB1);
	}
}

int main(void) {
	
	init_uart();
	
	DDRB |= (1<<PB1);
	
	state = waiting;
	input_state = low;
	
	sei();
	
	//speaker();
		
	while (1){ 
		
		
		
		if (state == waiting) {
			/*printf("Would you like to trigger the ultra-sonic ranger?\n");
			scanf("%s", input);
			if (strcmp(input, "y") == 0) {
				printf("Ranging...\n");*/
				state = set_trigger;
			}
		
		trigger();
		
		if(state == calculating) {
			if (ending_cnt > starting_cnt) {
				time_difference = ending_cnt - starting_cnt;
				} else {
				time_difference = (ending_cnt - starting_cnt) + 0xFFFF;
			}
			time_difference = (time_difference/65535)*realtime;
			distance = (unsigned short) ((time_difference*343.0)*100/2.0);
			distance = 1.0052*distance + 8.8767;
			printf("The distance is: %u centimeters\n", distance);
			state = set_trigger;
			TIFR1 = 0xFF;
			overflow = 0;
			TCNT1 = 0;
			TCCR1B = 0;
			TIMSK1 = 0;
			
			speaker();
		}
	
}
		}
