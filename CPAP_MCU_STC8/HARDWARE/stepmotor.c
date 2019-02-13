#include "stepmotor.h"
#include "timer.h"
#include "io.h"
#include "uart.h"
#include "cmd_process.h"
#include "management.h"

#include <stdio.h>

//void StepMotor_IsCW(void)
//{
//    return (!!(man.stepMotorStatus&0x40));
//}

void StepMotor_SetSpeed(uint8_t level)
{
    stepMotorSpeedIndex = level;
    T3H = timer_TH_TL[stepMotorSpeedIndex][0];
    T3L = timer_TH_TL[stepMotorSpeedIndex][1];
}

void StepMotor_CW(void)
{
    STM_DIR = 0;
    //TR0 = 1;
    T4T3M |= 0x08;
    man.stepMotorStatus |= 0xC0;
    //cDebug("StepMotor_CW\r\n");
}

void StepMotor_CCW(void)
{
    STM_DIR = 1;
    //TR0 = 1;
    T4T3M |= 0x08;
    man.stepMotorStatus |= 0x80;
    man.stepMotorStatus &= ~0x40;
    //cDebug("StepMotor_CCW\r\n");
}

void StepMotor_Step(uint16_t step)
{
    man.stepMotorStep = step;
    man.stepMotorStatus |= 0x20;   
}

void StepMotor_Stop(void)
{
    //TR0 = 0;
    T4T3M &= ~0x08;
    man.stepMotorStatus &= ~0xA0;
    //cDebug("StepMotor_Stop\r\n");
}

void HeatPlate_Adjust(uint8_t percent)
{
    if(percent > 100) percent = 100;

    if(percent == 0)
    {
        man.heatControl &= ~0x80;    
    }
    else
    {
        man.heatControl |= 0x80;
        man.plateHeatPWMPercent = percent;
    }
}

void HeatLine_Adjust(uint8_t percent)
{
    if(percent > 100) percent = 100;

    if(percent == 0)
    {
        man.heatControl &= ~0x08;    
    }
    else
    {
        man.heatControl |= 0x08;
        man.lineHeatPWMPercent = percent;
    }
}
