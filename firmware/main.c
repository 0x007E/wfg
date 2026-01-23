
#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart/uart.h"

volatile unsigned int tick_counter;
volatile unsigned int last_rise_time;
volatile unsigned int high_time;
volatile unsigned int period_ticks;

volatile unsigned char dutycycle;
volatile unsigned int frequency;

ISR(INT0_vect)
{
	sei();
	 unsigned int now = tick_counter;

	 if (PIND & (1<<PIND2))
	 {
		 last_rise_time = now;
	 }
	 else
	 {
		 unsigned int high_ticks = now - last_rise_time;

		 static unsigned int previous_rise_time = 0;
		 
		 period_ticks = last_rise_time - previous_rise_time;
		 previous_rise_time = last_rise_time;

		 high_time = high_ticks;
		 
		 if (period_ticks != 0)
		 {
			 dutycycle = (unsigned char)((high_time * 100UL) / period_ticks);
			 frequency = (unsigned int)(100000UL / period_ticks);
		 }
	 }
}

ISR(TIMER0_COMP_vect)
{
	tick_counter++;
}

static void external_init(void)
{
	DDRD &= ~(1<<PIND2);	// Input
	PORTD |= (1<<PIND2);	// Pull-up
	
	MCUCR = (1<<ISC00);		// Any logical change
	GICR = (1<<INT0);		// Enable interrupt
}

static void timer_init(void)
{
	OCR0 = 119;
	TCCR0 = (1<<WGM01) | (1<<CS00);
	TIMSK = (1<<OCIE0);
}

int main(void)
{
	external_init();
	timer_init();
	sei();
	
	uart_init();
    
    while (1) 
    {
		printf("Duty Cycle: %3u, Frequency: %5u   \r", dutycycle, frequency);
		
        _delay_ms(200);
    }
}

