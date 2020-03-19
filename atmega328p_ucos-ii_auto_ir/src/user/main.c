#include "user.h"

static OS_STK stack0[256];
// static OS_STK stack1[128];

static void tickInit( void )
{
	TCNT1 = 0;
	OCR1A = 15624;
	OCR1B = 0;
	TCCR1A = 0x00;
	TCCR1B = 0x0D;
	TCCR1C = 0x00;
	TIFR1 = ex(OCF1A);
	TIMSK1 = ex(OCIE1A);
}

int main(void)
{
	cli();
	OSInit();
	tickInit();

	enablePullup();
	usart0Init();

	ledInit();
	ir_init();

	OSTaskCreate(ir_demo, (void *)0, &stack0[255], 0);
	// OSTaskCreate(blink, (void *)0, &stack1[127], 1);

	OSStart();

	return 0;
}
