/*
 * GccApplication10.c
 *
 * Created: 2018-03-15 11:51:31 PM
 * Author : Matt
 */ 

#include <avr/io.h>


int main(void)
{
    DDRB = (1<<PB3);
	TCNT0 = 0;
	
	TCCR0A = (1<<WGM01) | (1<<COM0A0);
	TCCR0B = (1<<CS02)  | (1<<CS00);
	
	OCR0A = 14;
	/* Replace with your application code */
    while (1) 
    {
    }
}

