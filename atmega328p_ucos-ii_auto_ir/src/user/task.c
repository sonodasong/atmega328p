#include "task.h"

#define TEMP_DEFAULT		26
#define TEMP_SCALE			131
#define TIME_DRY			600
#define AC_OFF				0x00
#define AC_DRY				0x01
#define AC_COOL				0x02

static void acMode(uint8 mode);

volatile static int16 tempHigh = (TEMP_DEFAULT + 1) * TEMP_SCALE;
volatile static int16 tempLow = (TEMP_DEFAULT - 1) * TEMP_SCALE;
volatile static uint8 dryPeriod = 4;

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

void autoAc(void *pdata)
{
	uint8 state;
	uint8 dryCnt;
	uint16 dryTime;
	int16 temperature;
	int16 temp;

	(void)pdata;
	dryCnt = 0;
	temperature = (int16) adcRead();
	if (temperature < tempLow) {
		state = AC_OFF;
		acMode(AC_OFF);
	} else {
		state = AC_DRY;
		acMode(AC_DRY);
		dryTime = 0;
		dryCnt++;
	}
	while (1) {
		temp = (int16) adcRead();
		temperature += (temp - temperature) / 64;
		// usart0Printf("%d %d %d\r\n", temperature / TEMP_SCALE, temp / TEMP_SCALE, state);
		if (state == AC_OFF) {
			if (temperature > tempHigh) {
				if (dryCnt % dryPeriod == 0) {
					state = AC_DRY;
					acMode(AC_DRY);
					dryTime = 0;
				} else {
					state = AC_COOL;
					acMode(AC_COOL);
				}
				dryCnt++;
			}
		} else if (state == AC_DRY) {
			if (temperature < tempLow) {
				state = AC_OFF;
				acMode(AC_OFF);
			} else {
				dryTime++;
				if (dryTime >= TIME_DRY) {
					state = AC_COOL;
					acMode(AC_COOL);
				}
			}
		} else if (state == AC_COOL) {
			if (temperature < tempLow) {
				state = AC_OFF;
				acMode(AC_OFF);
			}
		}
		OSTimeDly(1);
	}
}

void autoAcInterface(void *pdata)
{
	char *str;
	int16 temp;

	(void)pdata;
	while (1) {
		usart0Read(&str);
		if (str[0] == 't') {
			temp = (str[1] - '0') * 10 + str[2] - '0';
			tempHigh = (temp + 1) * TEMP_SCALE;
			tempLow = (temp - 1) * TEMP_SCALE;
			usart0Printf("Temperature set at %d\r\n", temp);
		} else if (str[0] == 'd') {
			dryPeriod = str[1] - '0';
			usart0Printf("Dry period set at %d\r\n", dryPeriod);
		}
	}
}

static uint8 acOff[14] = {
	0x23, 0xCB, 0x26, 0x01,
	0x00, 0x20, 0x02, 0x07,
	0x3A, 0x00, 0x00, 0x00,
	0x00, 0x78
};

static uint8 acDry[14] = {
	0x23, 0xCB, 0x26, 0x01,
	0x00, 0x24, 0x02, 0x07,
	0x3A, 0x00, 0x00, 0x00,
	0x00, 0x7C
};

/* cool 16 celsius */
static uint8 acCool[14] = {
	0x23, 0xCB, 0x26, 0x01,
	0x00, 0x24, 0x03, 0x0F,
	0x3A, 0x00, 0x00, 0x00,
	0x00, 0x85
};

static void acMode(uint8 mode)
{
	uint8 *code;
	char *msg;

	if (mode == AC_DRY) {
		code = acDry;
		msg = "DRY";
	} else if (mode == AC_COOL) {
		code = acCool;
		msg = "COOL";
	} else {
		code = acOff;
		msg = "OFF";
	}
	OSTimeDly(1);
	irSend(code, 14);
	usart0Printf("Enter AC %s Mode\r\n", msg);
}
