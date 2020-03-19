#include "adc.h"

void adcInit(void)
{
	ADMUX = 0x40;
	ADCSRA = 0x87;
	ADCSRB = 0x00;
	DIDR0 = 0x01;
}

uint16 adcRead(void)
{
	uint16 result;
	uint8 low;

	ADCSRA = 0xC7;
	while (!(ADCSRA & ex(4)));
	low = ADCL;
	result = ADCH;
	result = (result << 8) | low;
	ADCSRA = 0x97;
	return result;
}
