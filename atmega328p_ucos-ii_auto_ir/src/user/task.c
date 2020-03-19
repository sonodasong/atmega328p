#include "task.h"

static uint8 ac_off[14] = {
	0x23, 0xCB, 0x26, 0x01,
	0x00, 0x20, 0x02, 0x07,
	0x3A, 0x00, 0x00, 0x00,
	0x00, 0x78
};

static uint8 dry[14] = {
	0x23, 0xCB, 0x26, 0x01,
	0x00, 0x24, 0x02, 0x07,
	0x3A, 0x00, 0x00, 0x00,
	0x00, 0x7C
};

static uint8 cool_16[14] = {
	0x23, 0xCB, 0x26, 0x01,
	0x00, 0x24, 0x03, 0x0F,
	0x3A, 0x00, 0x00, 0x00,
	0x00, 0x85
};

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

void ir_demo(void *pdata)
{
	char *str;

	(void)pdata;
	while (1) {
		usart0Read(&str);
		if (str[0] == '1') {
			OSTimeDly(1);
			ir_send(cool_16, 14);
		} else if (str[0] == '2') {
			OSTimeDly(1);
			ir_send(dry, 14);
		} else if (str[0] == '3') {
			OSTimeDly(1);
			ir_send(ac_off, 14);
		}
		usart0Printf("%s\r\n", str);
	}
}

