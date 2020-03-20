#include "task.h"

#define TEMP_LOW		54
#define TEMP_HIGH		55
#define TIME_DRY		300
#define AC_OFF			0x00
#define AC_DRY			0x01
#define AC_COOL			0x02

static void ac_mode(uint8 mode);

volatile static uint16 temp;

void blink(void *pdata)
{
	(void)pdata;
	while (1) {
		ledOn();
		OSTimeDly(1);
		ledOff();
		OSTimeDly(1);
	}
}

void serial(void *pdata)
{
	char *str;

	(void)pdata;
	while (1) {
		usart0Read(&str);
		usart0Printf("%s\r\n", str);
	}
}

void auto_ac(void *pdata)
{
	uint8 state;
	uint16 dry;

	(void)pdata;
	temp = adcRead();
	if (temp < TEMP_LOW) {
		state = AC_OFF;
		ac_mode(AC_OFF);
	} else {
		state = AC_DRY;
		ac_mode(AC_DRY);
		dry = 0;
	}
	while (1) {
		temp = adcRead();
		usart0Printf("%d %d\r\n", temp, state);
		if (state == AC_OFF) {
			if (temp > TEMP_HIGH) {
				state = AC_DRY;
				ac_mode(AC_DRY);
				dry = 0;
			}
		} else if (state == AC_DRY) {
			if (temp < TEMP_LOW) {
				state = AC_OFF;
				ac_mode(AC_OFF);
			} else {
				dry += 1;
				if (dry >= TIME_DRY) {
					state = AC_COOL;
					ac_mode(AC_COOL);
				}
			}
		} else if (state == AC_COOL) {
			if (temp < TEMP_LOW) {
				state = AC_OFF;
				ac_mode(AC_OFF);
			}
		}
		OSTimeDly(1);
	}
}

void auto_ac_debug(void *pdata)
{
	char *str;

	(void)pdata;
	while (1) {
		usart0Read(&str);
		temp = (str[0] - '0') * 10 + str[1] - '0';
		usart0Printf("%d\r\n", temp);
	}
}

static uint8 ac_off[14] = {
	0x23, 0xCB, 0x26, 0x01,
	0x00, 0x20, 0x02, 0x07,
	0x3A, 0x00, 0x00, 0x00,
	0x00, 0x78
};

static uint8 ac_dry[14] = {
	0x23, 0xCB, 0x26, 0x01,
	0x00, 0x24, 0x02, 0x07,
	0x3A, 0x00, 0x00, 0x00,
	0x00, 0x7C
};

/* cool 16 celsius */
static uint8 ac_cool[14] = {
	0x23, 0xCB, 0x26, 0x01,
	0x00, 0x24, 0x03, 0x0F,
	0x3A, 0x00, 0x00, 0x00,
	0x00, 0x85
};

static void ac_mode(uint8 mode)
{
	uint8 *code;
	char *msg;

	if (mode == AC_DRY) {
		code = ac_dry;
		msg = "DRY";
	} else if (mode == AC_COOL) {
		code = ac_cool;
		msg = "COOL";
	} else {
		code = ac_off;
		msg = "OFF";
	}
	OSTimeDly(1);
	ir_send(code, 14);
	// OSTimeDly(1);
	// ir_send(code, 14);
	usart0Printf("Enter AC %s Mode\r\n", msg);
}
