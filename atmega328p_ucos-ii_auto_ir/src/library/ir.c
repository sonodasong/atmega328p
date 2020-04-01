#include "ir_port.h"
#include "ir.h"

static void irHeader(void);
static void ir0(void);
static void ir1(void);
static void irEnd(void);
static BOOLEAN irNext(void);

static uint8 *irBuf;
static uint8 irLen;
static uint8 irByte;
static uint8 irBit;
static uint8 irState;
static uint8 irPulseOn;
static uint8 irPulseOff;

static OS_EVENT *irRdy;

void irInit(void)
{
	irPortInit();
	irRdy = OSSemCreate(0);
}

void irSend(uint8 *buf, uint8 len)
{
	INT8U err;

	irBuf = buf;
	irLen = len;
	irByte = 0;
	irBit = 0;
	irPortOn();
	irHeader();
	OSSemPend(irRdy, IR_TIMEOUT, &err);
}

void irHandler(void)
{
	if (irState == 0) {
		irPulseOn -= 1;
		if (irPulseOn == 0) {
			irState = 1;
			irPortPulseOff();
		}
	} else if (irState == 1) {
		irPulseOff -= 1;
		if (irPulseOff == 0) {
			if (!irNext()) {
				irEnd();
			}
		}
	} else if (irState == 2) {
		irPulseOn -= 1;
		if (irPulseOn == 0) {
			irState = 1;
			irPortPulseOff();
			irPortOff();
			OSIntEnter();
			OSSemPost(irRdy);
			OSIntExit();
		}
	}
}

static void irHeader(void)
{
	irState = 0;
	irPortPulseOn();
	irPulseOn = 8;
	irPulseOff = 4;
}

static void ir0(void)
{
	irState = 0;
	irPortPulseOn();
	irPulseOn = 1;
	irPulseOff = 1;
}

static void ir1(void)
{
	irState = 0;
	irPortPulseOn();
	irPulseOn = 1;
	irPulseOff = 3;
}

static void irEnd(void)
{
	irState = 2;
	irPortPulseOn();
	irPulseOn = 1;
	irPulseOff = 0;
}

static BOOLEAN irNext(void)
{
	if (irByte == irLen) {
		return FALSE;
	} else {
		if (irBuf[irByte] & ex(irBit)) {
			ir1();
		} else {
			ir0();
		}
		irBit += 1;
		if (irBit == 8) {
			irBit = 0;
			irByte += 1;
		}
		return TRUE;
	}
}
