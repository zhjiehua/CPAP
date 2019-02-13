#include "PID.h"

#define KP	8
#define KI	0
#define KD	0

static int32_t Value2Point(int32_t Val)
{
    return Val;
}

void PID_Init(PID_TypeDef *p, float32_t Kp, float32_t Ki, float32_t Kd)
{
	//qDebug("PID_Init!\n");
	p->Proportion = Kp;
	p->Integral = Ki;
	p->Derivative = Kd;

	//p->Proportion = 0.2;
	//p->Integral = 0.015;
	//p->Derivative = 0.2;

	p->CurrError = 0;
	p->PrevError = 0;
#if PID_POSITION_TYPE
	p->SumError = 0;
#endif
#if PID_INCREAMENT_TYPE
	p->LastError = 0;
#endif

#if PID_INTEGRAL_SAPARATION_TYPE
	p->IntSaparFlag = 0;
	p->SumError = 0;
#endif

#if PID_INTEGRAL_ANTISATURATION_TYPE
	p->PointMax = 210; //最大值
	p->PointMin = -10; //最小值
	p->IntSaparFlag = 0; //积分分离标志
	p->SumError = 0;//累计误差
#endif

#if PID_INTEGRAL_VARIAL_TYPE
	p->IntVarFactor = 0; //积分分离标志
	p->SumError = 0;//累计误差
#endif

	p->SetPoint = 0;
	p->ActualPoint = 0;
	p->Value = 0;
	p->Value2Point = Value2Point;

	//qDebug("PID_Init finish!\n");
}

void PID_UpdateSetPoint(PID_TypeDef *p, int32_t Point)
{
	p->SetPoint = Point;	
}

void PID_UpdateActualPoint(PID_TypeDef *p, int32_t Point)
{
	p->ActualPoint = Point;	
}

#if	PID_POSITION_TYPE  //位置型PID
int32_t PID_Calc(PID_TypeDef *p)
{
	p->CurrError = p->SetPoint - p->ActualPoint;

	p->SumError += p->CurrError;
	p->Value = (p->Proportion * p->CurrError)
		+ (p->Integral * p->SumError)
		+ (p->Derivative * (p->CurrError - p->PrevError));

	p->PrevError = p->CurrError;

	return (p->Value2Point(p->Value));
}
#endif

#if PID_INCREAMENT_TYPE  //增量型PID
int32_t PID_Calc(PID_TypeDef *p)
{
	int32_t temp;
	p->CurrError = p->SetPoint - p->ActualPoint;

	temp = (p->Proportion * (p->CurrError - p->PrevError))
		+ (p->Integral * p->CurrError)
		+ (p->Derivative * (p->CurrError - 2*p->PrevError + p->LastError));

	p->Value += temp;//增量型，需要更新PID值
	p->LastError = p->PrevError;
	p->PrevError = p->CurrError;

	return (p->Value2Point(p->Value));
	//return temp;
	//return p->Value;
}
#endif

#if	PID_INTEGRAL_SAPARATION_TYPE  //积分分离型PID
int32_t PID_Calc(PID_TypeDef *p)
{
	p->CurrError = p->SetPoint - p->ActualPoint;

	if(abs(p->CurrError) > (p->SetPoint*0.75))//INTEGRAL_SAPARATION_THRESHOLD
		p->IntSaparFlag = 0;
	else
	{
		p->IntSaparFlag = 1;
		p->SumError += p->CurrError;
	}
	p->Value = (p->Proportion * p->CurrError)
		+ (p->Integral * p->SumError)*p->IntSaparFlag
		+ (p->Derivative * (p->CurrError - p->PrevError));

	p->PrevError = p->CurrError;

	return (p->Value2Point(p->Value));
}
#endif

#if	PID_INTEGRAL_ANTISATURATION_TYPE  //抗饱和积分PID
int32_t PID_Calc(PID_TypeDef *p)
{
	p->CurrError = p->SetPoint - p->ActualPoint;

	if(p->ActualPoint > p->PointMax)
	{
		if(abs(p->CurrError) > INTEGRAL_SAPARATION_THRESHOLD)//积分分离过程
			p->IntSaparFlag = 0;
		else
		{
			p->IntSaparFlag = 1;
			if(p->CurrError < 0)
				p->SumError += p->CurrError;
		}
	}
	else if(p->ActualPoint < p->PointMin)
	{
		if(abs(p->CurrError) > INTEGRAL_SAPARATION_THRESHOLD)//积分分离过程
			p->IntSaparFlag = 0;
		else
		{
			p->IntSaparFlag = 1;
			if(p->CurrError > 0)
				p->SumError += p->CurrError;
		}	
	}
	else
	{
		if(abs(p->CurrError) > INTEGRAL_SAPARATION_THRESHOLD)//积分分离过程
			p->IntSaparFlag = 0;
		else
		{
			p->IntSaparFlag = 1;
			p->SumError += p->CurrError;
		}
	}
	
	p->Value = (p->Proportion * p->CurrError)
		+ (p->Integral * p->SumError)*p->IntSaparFlag
		+ (p->Derivative * (p->CurrError - p->PrevError));

	p->PrevError = p->CurrError;

	return (p->Value2Point(p->Value));
}
#endif

#if	PID_INTEGRAL_VARIAL_TYPE  //变积分型PID
int32_t PID_Calc(PID_TypeDef *p)
{
	p->CurrError = p->SetPoint - p->ActualPoint;

	if(abs(p->CurrError) > p->SetPoint)//INTEGRAL_VARIATION_HIGH_THRESHOLD
		p->IntVarFactor = 0;
	else if(abs(p->CurrError) < (p->SetPoint*0.9))//INTEGRAL_VARIATION_LOW_THRESHOLD
	{
		p->IntVarFactor = 1;
		p->SumError += p->CurrError;
	}
	else
	{
		p->IntVarFactor = (p->SetPoint - abs(p->CurrError))/(p->SetPoint*0.1);
		p->SumError += p->CurrError;
	}
	p->Value = (p->Proportion * p->CurrError)
		+ (p->Integral * p->SumError)*p->IntVarFactor
		+ (p->Derivative * (p->CurrError - p->PrevError));

	p->PrevError = p->CurrError;

	return (p->Value2Point(p->Value));
}
#endif
