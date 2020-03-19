#include "ir_port.h"

void ir_port_init(void)
{
	/* ir pulse initialization */
	OCR2A = 51;
	OCR2B = 26;
	TCCR2A = 0x33;
	TCCR2B = 0x08;
	TIMSK2 = 0x00;
	DDRD |= ex(3);

	/* ir initialization */
	OCR0A = 104;
	TCCR0A = 0x02;
	TCCR0B = 0x03;
	TIMSK0 = 0x00;
}

void ledOn(void);
void ledOff(void);

void ir_port_pulse_on(void)
{
	TCNT2 = 0;
	TCCR2B = 0x0A;
}

void ir_port_pulse_off(void)
{
	TCCR2B = 0x08;
}

void ir_port_on(void)
{
	TCNT0 = 0;
	TIFR0 = ex(OCF0A);
	TIMSK0 = ex(OCIE0A);
}

void ir_port_off(void)
{
	TIMSK0 = 0x00;
}

extern void ir_handler(void);

ISR(TIMER0_COMPA_vect)
{
	ir_handler();
}
