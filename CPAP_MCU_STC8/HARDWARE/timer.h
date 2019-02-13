#ifndef __TIMER_H__
#define __TIMER_H__

#include "common.h"

#define T100NS (65536-FOSC/12/10000) //100ns
#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode
#define T5MS (65536-5*FOSC/12/1000) //5ms

//#define TIMEOUT T100NS
#define TIMEOUT T1MS  //定时器重装载值
//#define TIMEOUT T5MS

#define BEEPER_PEROID 2700
#define HEAT_PEROID 1500

//-------------------------------------------------------------------------------
#define BASE_PEROID 1500

typedef struct
{
    uint8_t beeperPeroid;
}TimerMan_TypeDef;

extern uint8_t stepMotorSpeedIndex;
extern const uint8_t timer_TH_TL[7][2];

void Timer_Init(void);

void Delay1us();
void Delay10us();
void Delay100us();
void Delay500us();
void Delay1ms();
void Delay10ms();
void Delay100ms();


#endif