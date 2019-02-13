#ifndef __PID_H_
#define __PID_H_

//#include "stdlib.h"
#include "common.h"

//下面的2种PID算法为基本PID算法
#define PID_POSITION_TYPE	0	//位置型PID
#define PID_INCREAMENT_TYPE 1	//增量型PID

//下面的几种PID辅助算法都是用位置型PID为基本模板
#define PID_INTEGRAL_SAPARATION_TYPE	0	//积分分离式，用于解决目标点与当前点相差太大导致超调问题
#define PID_INTEGRAL_ANTISATURATION_TYPE	0	//抗饱和积分式，防止被控对象进入饱和区导致失控
//#define PID_TRAPEZOID_INTEGRAL_TYPE		0	//梯形积分式，提高积分精度，位置型改为p->SumError += (p->CurrError + p->PrevError)/2;
#define PID_INTEGRAL_VARIAL_TYPE		0	//变积分式，偏差越大，积分越慢；偏差越小，积分越快

//#define KP	0.3
//#define KI	0.6
//#define KD	0.5

#define INTEGRAL_SAPARATION_THRESHOLD	150 //积分分离阈值
#define INTEGRAL_VARIATION_LOW_THRESHOLD	180 //积分分离阈值
#define INTEGRAL_VARIATION_HIGH_THRESHOLD	200 //积分分离阈值

//typedef unsigned char uint8_t;
//typedef unsigned short uint16_t;
//typedef unsigned int uint32_t;
//typedef char int8_t;
//typedef short int16_t;
//typedef int int32_t;

typedef unsigned long int uint64_t;
typedef long int int64_t;

typedef float float32_t;
typedef double float64_t;

typedef struct
{
	float32_t Proportion;//比例常数
	float32_t Integral;//积分常数
	float32_t Derivative;//微分常数

	int32_t CurrError;//Error[0]
	int32_t PrevError;//Error[-1]
#if PID_POSITION_TYPE
	int64_t SumError;//累计误差
#elif PID_INCREAMENT_TYPE
	int32_t LastError;//Error[-2]
#endif

#if PID_INTEGRAL_SAPARATION_TYPE
	int8_t IntSaparFlag; //积分分离标志
	int64_t SumError;//累计误差
#endif

#if PID_INTEGRAL_ANTISATURATION_TYPE
	int32_t PointMax; //最大值
	int32_t PointMin; //最小值
	int8_t IntSaparFlag; //积分分离标志
	int64_t SumError;//累计误差
#endif

#if PID_INTEGRAL_VARIAL_TYPE
	int8_t IntVarFactor; //积分分离标志
	int64_t SumError;//累计误差
#endif

	int32_t	SetPoint;//目标点
	int32_t ActualPoint;//实际采集点，输入

	int32_t Value;//PID计算出来的值
	int32_t (*Value2Point)(int32_t v);//PID值与实际输出间的映射关系，输出
}PID_TypeDef;

extern PID_TypeDef *pPID;
void PID_Init(PID_TypeDef *p, float32_t Kp, float32_t Ki, float32_t Kd);
void PID_UpdateSetPoint(PID_TypeDef *p, int32_t Point);//更新目标点
void PID_UpdateActualPoint(PID_TypeDef *p, int32_t Point);//更新采集点
int32_t PID_Calc(PID_TypeDef *p); //PID核心函数

#endif
