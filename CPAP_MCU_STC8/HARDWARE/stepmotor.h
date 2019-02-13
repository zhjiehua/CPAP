#ifndef __STEPMOTOR_H__
#define __STEPMOTOR_H__

#include  "common.h"

void StepMotor_SetSpeed(uint8_t level);
void StepMotor_CW(void);
void StepMotor_CCW(void);
void StepMotor_Step(uint16_t step);
void StepMotor_Stop(void);

void HeatPlate_Adjust(uint8_t percent);
void HeatLine_Adjust(uint8_t percent);

#endif
