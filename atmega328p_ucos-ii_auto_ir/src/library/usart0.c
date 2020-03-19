#include "usart0.h"

#define usart0RxIntEnable()		UCSR0B |= ex(RXCIE0)
#define usart0RxIntDisable()	UCSR0B &= rex(RXCIE0)
#define usart0TxIntEnable()		UCSR0B |= ex(TXCIE0)
#define usart0TxIntDisable()	UCSR0B &= rex(TXCIE0)
#define usart0TxIntFlagClr()	UCSR0A |= ex(TXC0)

static char usart0RxBuf[ex(USART0_RX_Q_SIZE)][ex(USART0_RX_SIZE)];
static uint8 usart0RxCnt;
static uint8 usart0RxQCnt;
static void *usart0RxQPtr[ex(USART0_RX_Q_SIZE)];
static OS_EVENT *usart0RxQ;

static char *usart0TxPtr;
static uint8 usart0TxCnt;
static OS_EVENT *usart0TxRdy;

void usart0Init(void)
{
	UCSR0A = 0x02;
	UCSR0B = 0x18;
	UCSR0C = 0x06;
	UBRR0 = 207;

	usart0RxIntEnable();
	usart0TxIntFlagClr();
	usart0TxIntEnable();

	usart0RxQ = OSQCreate(usart0RxQPtr, ex(USART0_RX_Q_SIZE));
	usart0RxCnt = 0;
	usart0RxQCnt = 0;
	usart0TxRdy = OSSemCreate(0);
}

INT8U usart0Read(char **str)
{
	INT8U err;

	*str = (char *)OSQPend(usart0RxQ, USART0_RX_TIMEOUT, &err);
	return err;
}

INT8U usart0Print(char *str)
{
	INT8U err;
	char temp;

	usart0TxPtr = str;
	usart0TxCnt = 0;
	temp = *str;
	if (temp != '\0') {
		UDR0 = temp;
		OSSemPend(usart0TxRdy, USART0_TX_TIMEOUT, &err);
	}
	return err;
}

#if USART0_ENABLE_PRINTF

#include <stdio.h>
#include <stdarg.h>

static char usart0TxBuf[ex(USART0_TX_SIZE)];

void usart0Printf(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vsprintf(usart0TxBuf, fmt, ap);
	va_end(ap);
	usart0Print(usart0TxBuf);
}

#endif

static void usart0RxHandler(void)
{
	char usart0RxChar;

	usart0RxChar = UDR0;
	if (usart0RxChar == USART0_RX_EOF) {
		usart0RxBuf[usart0RxQCnt][usart0RxCnt] = '\0';
		usart0RxCnt = 0;
		OSIntEnter();
		OSQPost(usart0RxQ, usart0RxBuf[usart0RxQCnt]);
		usart0RxQCnt++;
		usart0RxQCnt &= ex(USART0_RX_Q_SIZE) - 1;
		OSIntExit();
	} else {
		usart0RxBuf[usart0RxQCnt][usart0RxCnt] = usart0RxChar;
		usart0RxCnt++;
		usart0RxCnt &= ex(USART0_RX_SIZE) - 1;
	}
}

static void usart0TxHandler(void)
{
	usart0TxCnt++;
	if (*(usart0TxPtr + usart0TxCnt) == 0) {
		OSIntEnter();
		OSSemPost(usart0TxRdy);
		OSIntExit();
	} else {
		UDR0 = usart0TxPtr[usart0TxCnt];
	}
}

ISR(USART_RX_vect)
{
	usart0RxHandler();
}

ISR(USART_TX_vect)
{
	usart0TxHandler();
}
