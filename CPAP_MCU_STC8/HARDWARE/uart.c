#include "uart.h"
#include "io.h"
#include "timer.h"

#include "cmd_queue.h"

uint8_t user_cmd_buffer[CMD_MAX_SIZE];
uint8_t lcd_cmd_buffer[CMD_MAX_SIZE];

//标志串口正在发送数据
static bit Uart_Busy;
static bit Uart2_Busy;

//串口初始化，串口波特率发生器用的是定时器1
void Uart_Init(void)
{
	/************************串口1***********************************/
	PCON &= ~0x80;	//串口1波特率控制位，0：各模式波特率不加倍；1：模式1、2、3波特率加倍

#if (PARITYBIT == NONE_PARITY)
    SCON = 0x50;            //8-bit variable UART
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    SCON = 0xda;            //9-bit variable UART, parity bit initial to 1
#elif (PARITYBIT == SPACE_PARITY)
    SCON = 0xd2;            //9-bit variable UART, parity bit initial to 0
#endif

	TMOD &= ~0xF0;            //定时器1为定时器，模式0，即16位自动重装载模式

	AUXR |= 0x40;  //定时器1速度控制，0:12T模式，即CPU时钟12分频，1:1T模式，直接用CPU时钟
	AUXR &= ~0x20;  //串口1波特率6倍速，0：不倍速；1:6倍速
	AUXR &= ~0x01;  //串口1波特率发生器选择，0：定时器1；1：定时器2

	TL1 = (65536 - (FOSC/4/USERUART_BAUD));
	TH1 = (65536 - (FOSC/4/USERUART_BAUD))>>8;

	TR1 = 1;                //Timer1 start run

	ES = 1;                 //Enable UART1 interrupt

	/************************串口2***********************************/
#ifdef HARDWARE_VERSION_18A
	P_SW2 &= ~0x01;                              //RXD2/P1.0, TXD2/P1.1
#endif
#ifdef HARDWARE_VERSION_24A
    P_SW2 |= 0x01;                               //RXD2_2/P4.0, TXD2_2/P4.2
#endif

	S2CON = 0x50;  //8位可变波特率

	AUXR &= ~0x08; //定时器2定时模式
	AUXR |= 0x04; //定时器2速度控制，0:12T模式，即CPU时钟12分频，1:1T模式，直接用CPU时钟
	AUXR |= 0x10; //启动定时器2
	T2L = (65536 - (FOSC/4/LCDUART_BAUD));
	T2H = (65536 - (FOSC/4/LCDUART_BAUD)) >> 8;	
	IE2 |= 0x01; //允许串口2中断
}

/*----------------------------
UART interrupt service routine
----------------------------*/
void Uart_Isr() interrupt 4 using 1
{
	uint8_t Buf = SBUF;
    qsize pos;

    if (RI)
    {
        RI = 0;             //Clear receive interrupt flag

		//8位单片机中断函数不能调用函数
		{
			//pos = (user_que._head+1)%QUEUE_MAX_SIZE;
            pos = user_que._head+1;  //使用RTX tiny 后只能把pos定义放在外面，也使用不了%QUEUE_MAX_SIZE
            if(pos >= QUEUE_MAX_SIZE) pos = 0;

			if(pos != user_que._tail)//非满状态
			{
				user_que._data[user_que._head] = Buf;
				user_que._head = pos;
			}
		}

		//queue_push(&user_que, Buf);

    }
    if (TI)
    {
        TI = 0;             //Clear transmit interrupt flag
        Uart_Busy = 0;           //Clear transmit busy flag
    }
}

/*----------------------------
Send a byte data to UART
Input: Data (data to be sent)
Output:None
----------------------------*/
void Uart_SendData(uint8_t Data)
{
    while (Uart_Busy);           //Wait for the completion of the previous data is sent
    ACC = Data;              //Calculate the even parity bit P (PSW.0)
    if (P)                  //Set the parity bit according to P
    {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 0;            //Set parity bit to 0
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 1;            //Set parity bit to 1
#endif
    }
    else
    {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 1;            //Set parity bit to 1
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 0;            //Set parity bit to 0
#endif
    }
    Uart_Busy = 1;
    SBUF = ACC;             //Send data to UART buffer
}

/*----------------------------
Send a string to UART
Input: s (address of string)
Output:None
----------------------------*/
void Uart_SendString(uint8_t *s)
{
    while (*s)              //Check the end of the string
    {
        Uart_SendData(*s++);     //Send current char and increment string ptr
    }
}

//重写putchar函数
char putchar(char c)
{
    Uart_SendData(c);
    return c;
}

/****************************************************
**
**串口2
**
*****************************************************/
void Uart2Isr() interrupt 8 using 2
{
    if (S2CON & 0x02)
    {
        S2CON &= ~0x02;
        Uart2_Busy = 0;
    }
    if (S2CON & 0x01)
    {
        S2CON &= ~0x01;
        //buffer[wptr++] = S2BUF;
        //wptr &= 0x0f;

		{
			qsize pos = (lcd_que._head+1)%QUEUE_MAX_SIZE;
			if(pos!=lcd_que._tail)//非满状态
			{
				lcd_que._data[lcd_que._head] = S2BUF;
				lcd_que._head = pos;
			}
		}
    }
}

void Uart2Send(char dat)
{
    while (Uart2_Busy);
    Uart2_Busy = 1;
    S2BUF = dat;
}

void Uart2SendStr(char *p)
{
    while (*p)
    {
        Uart2Send(*p++);
    }
}
