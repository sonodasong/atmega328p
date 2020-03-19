#ifndef __IR_H__
#define __IR_H__

#include "define.h"

#define IR_TIMEOUT		0xFFFFFFFF

void ir_init(void);
void ir_send(uint8 *buf, uint8 len);

#endif
