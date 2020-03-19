#include "ir_port.h"
#include "ir.h"

static void ir_header(void);
static void ir_0(void);
static void ir_1(void);
static void ir_end(void);
static BOOLEAN ir_next(void);

static uint8 *ir_buf;
static uint8 ir_len;
static uint8 ir_byte;
static uint8 ir_bit;
static uint8 ir_state;
static uint8 ir_pulse_on;
static uint8 ir_pulse_off;

static OS_EVENT *ir_rdy;

void ir_init(void)
{
	ir_port_init();
	ir_rdy = OSSemCreate(0);
}

void ir_send(uint8 *buf, uint8 len)
{
	INT8U err;

	ir_buf = buf;
	ir_len = len;
	ir_byte = 0;
	ir_bit = 0;
	ir_port_on();
	ir_header();
	OSSemPend(ir_rdy, IR_TIMEOUT, &err);
}

void ir_handler(void)
{
	if (ir_state == 0) {
		ir_pulse_on -= 1;
		if (ir_pulse_on == 0) {
			ir_state = 1;
			ir_port_pulse_off();
		}
	} else if (ir_state == 1) {
		ir_pulse_off -= 1;
		if (ir_pulse_off == 0) {
			if (!ir_next()) {
				ir_end();
			}
		}
	} else if (ir_state == 2) {
		ir_pulse_on -= 1;
		if (ir_pulse_on == 0) {
			ir_state = 1;
			ir_port_pulse_off();
			ir_port_off();
			OSIntEnter();
			OSSemPost(ir_rdy);
			OSIntExit();
		}
	}
}

static void ir_header(void)
{
	ir_state = 0;
	ir_port_pulse_on();
	ir_pulse_on = 8;
	ir_pulse_off = 4;
}

static void ir_0(void)
{
	ir_state = 0;
	ir_port_pulse_on();
	ir_pulse_on = 1;
	ir_pulse_off = 1;
}

static void ir_1(void)
{
	ir_state = 0;
	ir_port_pulse_on();
	ir_pulse_on = 1;
	ir_pulse_off = 3;
}

static void ir_end(void)
{
	ir_state = 2;
	ir_port_pulse_on();
	ir_pulse_on = 1;
	ir_pulse_off = 0;
}

static BOOLEAN ir_next(void)
{
	if (ir_byte == ir_len) {
		return FALSE;
	} else {
		if (ir_buf[ir_byte] & ex(ir_bit)) {
			ir_1();
		} else {
			ir_0();
		}
		ir_bit += 1;
		if (ir_bit == 8) {
			ir_bit = 0;
			ir_byte += 1;
		}
		return TRUE;
	}
}
