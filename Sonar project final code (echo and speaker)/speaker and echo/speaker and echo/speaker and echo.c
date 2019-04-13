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
#include <util/delay.h>
#include <math.h>



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

int count=0;
int angle=0;

void timer(){
	
			// initialize counter
	
	TIMSK2 = (1<<TOIE2)|(1<<OCIE2A); 	// enables interrupt of timer
	TCNT2 = 0; 				// default value of timer
	OCR2A = 14; 			// value of compare register
	TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20); // pre-scale ck/1024
	
	
		
		
		
	}
			
			

	


void speaker() {
		DDRB = (1<<PB3) | (1<<PB1) ; // initialize PB3 and PB1  as output, speaker uses PB3
		TCNT0 = 0;// initialize counter
		TCCR0A = (1<<WGM01) | (1<<COM0A0); // CTC mode
		TCCR0B = (1<<CS02)  | (1<<CS00); //
		if ((distance >= 1 && distance < 36)){
			OCR0A = 27;// 261 Hz
			
		
		}
			if ((distance >=36 && distance <72)){
				OCR0A = 25; // 282Hz
				
	}
		
if ((distance >=72 && distance <109)){
	OCR0A = 23; //303 Hz
	
}
	
if ((distance >=109 && distance <145)){
	OCR0A = 21; //324 Hz
	
}

if ((distance >=145 && distance <181)){
	OCR0A = 20;//346 Hz
	
}

if ((distance >=181 && distance <218)){
	OCR0A = 19; //367 Hz
	 
}

if ((distance >=218 && distance <254)){
	OCR0A = 18; //388 Hz
	
}
	
if ((distance >=254 && distance <290)){
OCR0A = 17; //409 Hz

}
	
if ((distance >=290 && distance <327)){
	OCR0A = 16; //430 Hz
	
}
if ((distance >=327 && distance <363)){
	OCR0A = 15; //451 Hz
}
if ((distance = 0)){
	OCR0A = 27; //261 Hz
}else{
	
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


ISR (TIMER2_COMPA_vect) {
			TCNT2 = 0;// reset timer
	count++; 

		if (count>=4710){
			TCNT2 = 0;
			count = 0;
			
		}//increment counter every 1ms
}

ISR (TIMER2_OVF_vect){
	TCNT2 = 0; 
	count = 0;
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
	

	
	int angle;
	init_uart();
	
	DDRB |= (1<<PB1);
	
	state = waiting;
	input_state = low;
	
	
	
	
	sei();
	
	timer();
		
	while (1){ 
		
		
		
		if (state == waiting) {
		
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
		
		
				
				angle = (count*0.038);
				angle = angle - 90;
			
		
			if (distance <= 360){
			printf("The distance is: %u centimeters and the angle is %d degrees\n", distance, angle);
				speaker();
			}
			state = set_trigger;
			TIFR1 = 0xFF;
			overflow = 0;
			TCNT1 = 0;
			TCCR1B = 0;
			TIMSK1 = 0;
			
		
			
			
		
			
			
			
			if (distance < 350){
				PORTC = (1<<PC1);
			}else {
				PORTC = (1<<PC0);
			}
			
			
			}
}
		}
