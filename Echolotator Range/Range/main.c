/*
 * Range.c
 *
 * Created: 11/20/2017 11:11:36 AM
 * Author : Mark
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "USART0.h"


FILE *fio_0 = &usart0_Stream;

volatile unsigned char MIP;
volatile unsigned int ECHOHigh, ECHOLow, ECHOHighPipe;
volatile unsigned int TimeOutCnt,Tick;


/***************************************************************************************
When the echo length is longer than the counter duration, we use an additional byte to 
indicate how many times we reach the maximum value. 
***************************************************************************************/
ISR (TIMER1_OVF_vect) {	// For long ECHO's
	if(ECHOHighPipe >= 2) {
		TIMSK1 = 0;	// No further interrupts.
		TCCR1B = 0; // Stop Clock
		MIP = 0xFF;	// End Measurement
	}
	
	ECHOHighPipe++;	// Add 1 to High byte.
}

/***************************************************************************************
Interrupt service routine called when the input capture pin state is changed
***************************************************************************************/
ISR (TIMER1_CAPT_vect) {	// Start and Stop ECHO measurement;
	if((TCCR1B & (1<<ICES1)) != 0) { // a rising edge has been detected
		TCCR1B |= (1<<CS11);	// Start counting with ck/8;
		TCCR1B &= ~(1<<ICES1);  // Configure Negative Edge Capture for end of echo pulse.
	}
	
	else {						// a falling edge has been detected
		ECHOLow = TCNT1;
		ECHOHigh = ECHOHighPipe;
		TIMSK1 = (1<<OCIE1B);	// Enables the Compare B interrupt for POST Trigger Delay: Approx 10mS
		TCNT1 = 0;
	}
}


/***************************************************************************************
Interrupt service routine called when the counter 1 has reached the compare value
***************************************************************************************/
ISR (TIMER1_COMPB_vect) {	// Compare B: Post ECHO delay 10mS



	MIP = 0;	// End Measurement
}

/***************************************************************************************
Interrupt service routine called when the counter 1 has reached the compare value
***************************************************************************************/
ISR (TIMER1_COMPA_vect) {	// Compare A : End of Trigger Pulse
	PORTB &= ~(1<<PB1);
	TIMSK1 = (1<<ICIE1)|(1<<TOIE1); // enables the T/C1 Overflow and Capture interrupt;
	TCCR1B = (1<<ICES1);			// Set Positive edge for capture but Don't count yet


}

/******************************************************************************
******************************************************************************/
void Trigger( void ) {		// Config Timer 1 for 10 to 15uS pulse.
	if(MIP == 0) {	// Don't allow re-trigger.
		MIP = 1;				// Set Measurement in progress FLAG
		DDRB |= (1<<PB1);		// PB1 as Output for Trigger pulse.
		DDRD &= ~(1<<PD6);		// PD6 as Input for Input Capture (ECHO).
		
		TCNT1 = 0;				// Clear last Echo times.
		ECHOHighPipe = 0;
		
		OCR1B = 10100;			// 10 mS Post echo Delay
		OCR1A = 12;				// 10 us Trigger length.

		PORTB |= (1<<PB1);		// Start Pulse.

		TIFR1 = 0xFF;			//  Clear all timer interrupt flags
		TCCR1A = 0;   // Timer mode with Clear Output on Match
		TCCR1B = (1<<WGM12) | (1<<CS11);  // Counting with CKio/8 CTC Mode enabled
		TIMSK1 = (1<<OCIE1A);	// enables the T/C1 Overflow, Compare A, and Capture interrupt;
	}
	
}

/******************************************************************************
******************************************************************************/
int main(void){
	
	uint8_t key;
	
    init_uart0(95);

    sei();
	
	fprintf_P(fio_0,PSTR("Hello\n\n\r")	);

    while (1 == 1) {
	while(uart0_RxCount() == 0){};
	
	key = uart0_getc();
	
	switch(key) {
		case 13:
			fprintf_P(fio_0,PSTR("\n\r")	);
			break;

		case 'T':
		case 't':
			Trigger();
			while (MIP == 1){};
			fprintf_P(fio_0,PSTR("Echo is %d %d\n\n\r"),	ECHOHigh, ECHOLow);
			break;		
		
		}
    }
}

