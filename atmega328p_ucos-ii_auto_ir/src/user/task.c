#include "task.h"

#define TEMP_DEFAULT		26
#define TEMP_SCALE			131
#define TIME_DRY			600
#define AC_OFF				0x00
#define AC_DRY				0x01
#define AC_COOL				0x02

static void ac_mode(uint8 mode);

volatile static int16 temp_high = (TEMP_DEFAULT + 1) * TEMP_SCALE;
volatile static int16 temp_low = (TEMP_DEFAULT - 1) * TEMP_SCALE;
volatile static int16 temperature;

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
	int16 temp;

	(void)pdata;
	temperature = (int16) adcRead();
	if (temperature < temp_low) {
		state = AC_OFF;
		ac_mode(AC_OFF);
	} else {
		state = AC_DRY;
		ac_mode(AC_DRY);
		dry = 0;
	}
	while (1) {
		temp = (int16) adcRead();
		temperature += (temp - temperature) / 64;
		// usart0Printf("%d %d %d\r\n", temperature / TEMP_SCALE, temp / TEMP_SCALE, state);
		if (state == AC_OFF) {
			if (temperature > temp_high) {
				state = AC_DRY;
				ac_mode(AC_DRY);
				dry = 0;
			}
		} else if (state == AC_DRY) {
			if (temperature < temp_low) {
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
			if (temperature < temp_low) {
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
	int16 temp;

	(void)pdata;
	while (1) {
		usart0Read(&str);
		temp = (str[0] - '0') * 10 + str[1] - '0';
		temp_high = (temp + 1) * TEMP_SCALE;
		temp_low = (temp - 1) * TEMP_SCALE;
		usart0Printf("Temperature set at %d\r\n", temp);
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
	// usart0Printf("Enter AC %s Mode\r\n", msg);
}
