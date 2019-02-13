#ifndef __UART_H__
#define __UART_H__

#include "common.h"
#include "CPrintf.h"
#include <stdio.h>

#define USERUART_BAUD 115200           //UART baudrate
#define LCDUART_BAUD 115200

/*Define UART parity mode*/
#define NONE_PARITY     0   //None parity
#define ODD_PARITY      1   //Odd parity
#define EVEN_PARITY     2   //Even parity
#define MARK_PARITY     3   //Mark parity
#define SPACE_PARITY    4   //Space parity

#define PARITYBIT NONE_PARITY   //Testing even parity

extern uint8_t user_cmd_buffer[];
extern uint8_t lcd_cmd_buffer[];

void Uart_Init(void);
void Uart_SendData(uint8_t dat);
void Uart_SendString(uint8_t *s);

void Uart2Send(char dat);
void Uart2SendStr(char *p);

#endif