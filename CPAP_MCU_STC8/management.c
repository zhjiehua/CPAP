#include "management.h"
#include "stepmotor.h"
#include "timer.h"
#include "io.h"
#include "uart.h"
#include "cmd_process.h"
#include "misc.h"
#include "eeprom.h"
#include "cmd_queue.h"
#include "tm770x.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KP 9.0
#define KI 0.0
#define KD 9.0

#define SIZEOF(X) sizeof(X)/sizeof(X[0])

const Pt100_T_R_TypeDef Pt100_Table[] = 
{
    {20.0, 107.79},
    {30.0, 111.67},
    {40.0, 115.54},
    {50.0, 119.40},
};

const OxygenSensor_V_P_TypeDef OxygenSensor_Table[] = 
{
    //{21.0, 14.0},
    //{88.0, 65.0},
    {21.0, 14.3},
    {99.0, 67.5},
};

float Pt100_K[SIZEOF(Pt100_Table)-1];
float OxygenSensor_K[SIZEOF(OxygenSensor_Table)-1];

Man_TypeDef man;

float Abs(float a)
{
    if(a >= 0)
        return (a);
    else
        return (-a);
}

uint8_t _abs(int8_t a)
{
    if(a >= 0)
        return (a);
    else
        return (-a);
}

void InitMan(void)
{
    uint8_t i, j, temp0;
    float temp[9];

    //生成温度-电阻斜率表
    for(i=0;i<SIZEOF(Pt100_Table)-1;i++)
    {
        Pt100_K[i] = (Pt100_Table[i+1].temperature - Pt100_Table[i].temperature)
                    /(Pt100_Table[i+1].resistorKOhm - Pt100_Table[i].resistorKOhm);
        //cDebug("Pt100_K[%d] = %f\r\n", (int)i, Pt100_K[i]);    
    }

    for(i=0;i<SIZEOF(OxygenSensor_Table)-1;i++)
    {
        OxygenSensor_K[i] = (OxygenSensor_Table[i+1].oxygenPercent - OxygenSensor_Table[i].oxygenPercent)
                    /(OxygenSensor_Table[i+1].voltageMV - OxygenSensor_Table[i].voltageMV);
        //cDebug("OxygenSensor_K[%d] = %f\r\n", (int)i, OxygenSensor_K[i]);    
    }

    man.autoCalibFlag = 0;
    man.autoCalibState = 0;

    man.powerOn = 1;
    man.powerOnCnt = POWERONNOALARMTIME;
    man.beeperAlarm = 0;
    man.beeperTimes = 0;
    man.alarmStatus = 0;
    man.alarmStatusLast = 0;
    man.heatControl = 0;
    man.plateHeatCnt = 0;
    man.plateHeatPWMPercent = 0;
    man.lineHeatCnt = 0;
    man.lineHeatPWMPercent = 0;

    man.oxygenMode = MODE_NONE; //工作模式初始化

    man.startFlag = 0; //暂停状态

    man.comboKeyCalibCnt = 0;
    man.comboKeyAutoCalibCnt = 0;
    man.comboKeyAdjustCnt = 0;
    man.comboKeyDefaultResetCnt = 0;
    man.comboKeyModeChangeCnt = 0;
    man.comboKeyBeeperStopCnt = 0;
    man.comboKeyWorkModeCnt = 0;

    //校准参数初始化，非常重要
    man.pCalib = &man.innerOxygenCalibOffset;
    man.calibIndex = 0;

    man.pAdjustPID = &man.oxygenPID;
    man.adjustPIDIndex = 0;

    man.beeperStopFlag = 0;

    temp0 = IapRead(EEPROM_BASEADDR_DEFAULT); //读3次
    //cDebug("eeprom data = 0x%x\r\n", temp0);
    if(temp0 != 0xA5)
       temp0 = IapRead(EEPROM_BASEADDR_DEFAULT);
    if(temp0 != 0xA5)
       temp0 = IapRead(EEPROM_BASEADDR_DEFAULT);
    //if(IapRead(EEPROM_BASEADDR_DEFAULT) == 0xA5)
    if(temp0 == 0xA5)
	//if(0)
    {
        cDebug("Reading the calibration data from eeprom...\r\n");

        EEPROM_ReadBytes(EEPROM_BASEADDR_CALIB, (uint8_t*)(man.pCalib), sizeof(float)*10);
        
        EEPROM_ReadBytes(EEPROM_BASEADDR_ADJUST, (uint8_t*)temp, sizeof(float)*9);
        PID_Init(&(man.oxygenPID), temp[0], temp[1], temp[2]);
        PID_Init(&(man.plateHeatTemperPID), temp[3], temp[4], temp[5]);
        PID_Init(&(man.lineHeatTemperPID), temp[6], temp[7], temp[8]);
//        PID_Init(&(man.oxygenPID), KP, KI, KD);
//        PID_Init(&(man.plateHeatTemperPID), KP, KI, KD);
//        PID_Init(&(man.lineHeatTemperPID), KP, KI, KD);
        
        EEPROM_ReadBytes(EEPROM_BASEADDR_AUTOCALIB, (uint8_t*)(&man.autoCalibDate), 4);
        
        man.workMode = IapRead(EEPROM_BASEADDR_WORKMODE);   
    }
    else
    {
        cDebug("Reseting and saving the calibration data...\r\n");

//        IapErase(EEPROM_BASEADDR_DEFAULT);
//        os_wait(K_TMO, 2, 0); //擦除需要等待4~6ms，这里等待10ms
//        IapErase(EEPROM_BASEADDR_CALIB);
//        os_wait(K_TMO, 2, 0); //擦除需要等待4~6ms，这里等待10ms

        man.innerOxygenCalibOffset = 0.0;
        man.innerOxygenCalibRatio = 1.0;
        man.maskOxygenCalibOffset = 0.0;
        man.maskOxygenCalibRatio = 1.0;
        man.innerTemperCalibOffset = 0.0;
        man.innerTemperCalibRatio = 1.0;
        man.cubeTemperCalibOffset = 0.0;
        man.cubeTemperCalibRatio = 1.0;
        man.maskTemperCalibOffset = 0.0;
        man.maskTemperCalibRatio = 1.0;

        //PID参数初始化
        PID_Init(&(man.oxygenPID), KP, KI, KD);
        PID_Init(&(man.plateHeatTemperPID), KP, KI, KD);
        PID_Init(&(man.lineHeatTemperPID), KP, KI, KD);
        temp[0] = KP;
        temp[1] = KI;
        temp[2] = KD;
        temp[3] = KP;
        temp[4] = KI;
        temp[5] = KD;
        temp[6] = KP;
        temp[7] = KI;
        temp[8] = KD;
        EEPROM_WriteBytes(EEPROM_BASEADDR_ADJUST, (uint8_t*)temp, sizeof(float)*9);    

        IapProgram(EEPROM_BASEADDR_DEFAULT, 0xA5);
        EEPROM_WriteBytes(EEPROM_BASEADDR_CALIB, (uint8_t*)(man.pCalib), sizeof(float)*10);

        SetRTC(18, 8, 1, 12, 0, 0); //设置RTC时间
        man.autoCalibDate = 18*365+8*12+1; //6667
        EEPROM_WriteBytes(EEPROM_BASEADDR_AUTOCALIB, (uint8_t*)(&man.autoCalibDate), 4);
        
        man.workMode = 0;
        IapProgram(EEPROM_BASEADDR_WORKMODE, man.workMode); 
    }

	//强制1模式
	man.workMode = 1;

    //页面数据初始化
	man.iSetupOxygen = 40;
    man.fSetupOxygen = man.iSetupOxygen;
	man.fSetupTemper = 37.0;
    //man.fSetupTemper = 30.0;

#if WORKMODE_KEYCHANGE    
    if(man.workMode == 1) //二合一
        man.pageModeSelect = PAGE_MODESELECT;
    else
        man.pageModeSelect = PAGE_MODESELECT_3;
    man.prevPage = man.pageModeSelect;
    man.curPage = man.pageModeSelect;
#else
    man.prevPage = PAGE_MODESELECT;
    man.curPage = PAGE_MODESELECT;
#endif

//    cDebug("man.innerOxygenCalibOffset = %f\r\n", man.innerOxygenCalibOffset);
//    cDebug("man.innerOxygenCalibRatio = %f\r\n", man.innerOxygenCalibRatio);
//    cDebug("man.maskOxygenCalibOffset = %f\r\n", man.maskOxygenCalibOffset);
//    cDebug("man.maskOxygenCalibRatio = %f\r\n", man.maskOxygenCalibRatio);
//    cDebug("man.innerTemperCalibOffset = %f\r\n", man.innerTemperCalibOffset);
//    cDebug("man.innerTemperCalibRatio = %f\r\n", man.innerTemperCalibRatio);
//    cDebug("man.cubeTemperCalibOffset = %f\r\n", man.cubeTemperCalibOffset);
//    cDebug("man.cubeTemperCalibRatio = %f\r\n", man.cubeTemperCalibRatio);
//    cDebug("man.maskTemperCalibOffset = %f\r\n", man.maskTemperCalibOffset);
//    cDebug("man.maskTemperCalibRatio = %f\r\n", man.maskTemperCalibRatio);
//
//    cDebug("man.oxygenPID = %f, %f, %f\r\n", man.oxygenPID.Proportion, man.oxygenPID.Integral, man.oxygenPID.Derivative);
//    cDebug("man.plateHeatTemperPID = %f, %f, %f\r\n", man.plateHeatTemperPID.Proportion, man.plateHeatTemperPID.Integral, man.plateHeatTemperPID.Derivative);
//    cDebug("man.lineHeatTemperPID = %f, %f, %f\r\n", man.lineHeatTemperPID.Proportion, man.lineHeatTemperPID.Integral, man.lineHeatTemperPID.Derivative);
//    
    
    cDebug("man.autoCalibDate = %ld\r\n", man.autoCalibDate);
    cDebug("man.workMode = %d\r\n", (int)man.workMode);

#if 0
    {
        uint8_t readADC[4];
        uint8_t i;

        for(i=0;i<5;i++)
        {
            readADC[0] = 0;
            readADC[1] = 0;
            readADC[2] = 0;
            readADC[3] = 0;
            TM770X_ReadCalibData(i, 0, readADC);
            Uart_SendData(i); 
            Uart_SendData(readADC[0]);
            Uart_SendData(readADC[1]);
            Uart_SendData(readADC[2]);
            Uart_SendData(readADC[3]);

            readADC[0] = 0;
            readADC[1] = 0;
            readADC[2] = 0;
            readADC[3] = 0;
            TM770X_ReadCalibData(i, 1, readADC);
            Uart_SendData(i); 
            Uart_SendData(readADC[0]);
            Uart_SendData(readADC[1]);
            Uart_SendData(readADC[2]);
            Uart_SendData(readADC[3]);
        }
    }
#endif

//    {
//        int16_t i, j;
//        uint8_t temp;
//        i = 1;
//        j = 5;
//        temp = Abs(i-j);
//        cDebug("abs() test = %d\r\n", (int)temp);
//    }
}

#define THRESHOLD   5.0

void OxygenPercentControl(void)
{
    //if(abs(man.fSetupOxygen - man.fCurInnerOxygen) > 15.0)
    if((man.fSetupOxygen >= man.fCurInnerOxygen) && ((man.fSetupOxygen - man.fCurInnerOxygen) > THRESHOLD)
        || (man.fSetupOxygen < man.fCurInnerOxygen) && ((man.fCurInnerOxygen - man.fSetupOxygen) > THRESHOLD))
    {
        StepMotor_SetSpeed(2);
        if(man.fSetupOxygen > man.fCurInnerOxygen)
        {
            StepMotor_CCW();
            //cDebug("%f ccw\r\n", THRESHOLD);
        }
        else
        {
            StepMotor_CW();
            //cDebug("%f cw\r\n", THRESHOLD);
        }
    }
    else if((man.fSetupOxygen >= man.fCurInnerOxygen) && ((man.fSetupOxygen - man.fCurInnerOxygen) > THRESHOLD/2)
        || (man.fSetupOxygen < man.fCurInnerOxygen) && ((man.fCurInnerOxygen - man.fSetupOxygen) > THRESHOLD/2))
    {
        StepMotor_SetSpeed(0);
        if(man.fSetupOxygen > man.fCurInnerOxygen)
        {
            StepMotor_Step(5);
            StepMotor_CCW();
            //cDebug("%f ccw\r\n", THRESHOLD/2);
        }
        else
        {
            StepMotor_Step(5);
            StepMotor_CW();
            //cDebug("%f cw\r\n", THRESHOLD/2);
        }    
    }
    else if((man.fSetupOxygen >= man.fCurInnerOxygen) && ((man.fSetupOxygen - man.fCurInnerOxygen) > THRESHOLD/5)
        || (man.fSetupOxygen < man.fCurInnerOxygen) && ((man.fCurInnerOxygen - man.fSetupOxygen) > THRESHOLD/5))
    {
        os_wait(K_TMO, 50, 0);                               

        StepMotor_SetSpeed(0);
        if(man.fSetupOxygen > man.fCurInnerOxygen)
        {
            StepMotor_Step(1);
            StepMotor_CCW();
            //cDebug("%f ccw\r\n", THRESHOLD/5);
        }
        else
        {
            StepMotor_Step(1);
            StepMotor_CW();
            //cDebug("%f cw\r\n", THRESHOLD/5);
        }    
    }
    else if((man.fSetupOxygen >= man.fCurInnerOxygen) && ((man.fSetupOxygen - man.fCurInnerOxygen) > THRESHOLD/20)
        || (man.fSetupOxygen < man.fCurInnerOxygen) && ((man.fCurInnerOxygen - man.fSetupOxygen) > THRESHOLD/20))
    {
        os_wait(K_TMO, 100, 0);                               

        StepMotor_SetSpeed(0);
        if(man.fSetupOxygen > man.fCurInnerOxygen)
        {
            StepMotor_Step(1);
            StepMotor_CCW();
            //cDebug("%f ccw\r\n", THRESHOLD/20);
        }
        else
        {
            StepMotor_Step(1);
            StepMotor_CW();
            //cDebug("%f cw\r\n", THRESHOLD/20);
        }
    }
    else
    {
        if(man.fSetupOxygen == man.fCurInnerOxygen)
            StepMotor_Stop();

        os_wait(K_TMO, 200, 0);                               

        StepMotor_SetSpeed(0);
        if(man.fSetupOxygen > man.fCurInnerOxygen)
        {
            StepMotor_Step(1);
            StepMotor_CCW();
            //cDebug("<%f ccw\r\n", THRESHOLD/20);
        }
        else
        {
            StepMotor_Step(1);
            StepMotor_CW();
            //cDebug("<%f cw\r\n", THRESHOLD/20);
        }    
    }
}

void TemperControl(void)
{
    uint8_t pwmPercent;
	int32_t pidOut;
	
#if 1
    //加热盘
    PID_UpdateActualPoint(&(man.plateHeatTemperPID), man.curInnerTemper*100);
	pidOut = PID_Calc(&(man.plateHeatTemperPID));
    //cDebug("pidOut1 = %d\r\n", (int)pidOut);

    if(Abs(man.curInnerTemper - man.fSetupTemper + 3.0) < 0.2)
//    if((man.curInnerTemper < (man.fSetupTemper - 3.0)) && ((man.fSetupTemper - 3.0 - man.curInnerTemper) < 0.2)
//        || (man.curInnerTemper >= (man.fSetupTemper - 3.0)) && ((man.curInnerTemper - man.curInnerTemper + 3.0) < 0.2))
    {
        pwmPercent = pidOut/100+40+(man.fSetupTemper - 30.0)*2;
        //cDebug("man.curInnerTemper = %f\r\n", man.curInnerTemper);
        //cDebug("man.fSetupTemper = %f\r\n", man.fSetupTemper); 
    }       
	else 
    {
		if(pidOut < 0)
			pwmPercent = 0;
		else
			pwmPercent = 100;
    }
    //cDebug("HeatPlate_Adjust = %d\r\n", (int)pwmPercent);
	
	if(pwmPercent >= 100)
		pwmPercent = 100;
	else if(pwmPercent <= 0)
		pwmPercent = 0;
	
	HeatPlate_Adjust(pwmPercent);
#endif
    //==================================================================================

#if 1
    //加热丝
    PID_UpdateActualPoint(&(man.lineHeatTemperPID), man.curCubeTemper*100);
	pidOut = PID_Calc(&(man.lineHeatTemperPID));
    //cDebug("pidOut2 = %d\r\n", (int)(pidOut));

#if 0
    if(Abs(man.curCubeTemper - man.fSetupTemper) < 0.5)
    {
        //pwmPercent = pidOut/100;
        pwmPercent = pidOut/100 + 20 + (man.fSetupTemper - 30.0)*2;
        //cDebug("man.fSetupTemper = %f\r\n", man.fSetupTemper);
//    if((man.fSetupTemper > man.curCubeTemper) && ((man.fSetupTemper - man.curCubeTemper) < 0.1))
//            pwmPercent = pidOut/10+2;
    }
	else 
    {
		if(pidOut < 0)
			pwmPercent = 0;
		else
			pwmPercent = 100;
    }
#else
    if(man.curCubeTemper > man.fSetupTemper)
    {
        if((man.curCubeTemper - man.fSetupTemper) > 0.1)
            pwmPercent = 0;
        else
            //pwmPercent = pidOut/100+2;
            pwmPercent = pidOut/100+6+(man.fSetupTemper - 30.0)*2;
    }
    else
    {
        if((man.fSetupTemper - man.curCubeTemper) > 3.5)
        {
            pwmPercent = 100;
            //cDebug("3.5 HL = %d\r\n", (int)pwmPercent);
        }
        else if((man.fSetupTemper - man.curCubeTemper) > 2.5)
        {
            pwmPercent = pidOut/100+80;
            //cDebug("2.5 HL = %d\r\n", (int)pwmPercent);
        }
        else if((man.fSetupTemper - man.curCubeTemper) > 1.5)
        {
            pwmPercent = pidOut/100+60;
            //cDebug("1.5 HL = %d\r\n", (int)pwmPercent);
        }
        else if((man.fSetupTemper - man.curCubeTemper) > 0.5)
        {
            pwmPercent = pidOut/100+30;
            //cDebug("0.5 HL = %d\r\n", (int)pwmPercent);
        }
        else
        {
            pwmPercent = pidOut/100+10;
            //pwmPercent = pidOut/100+6;
            //cDebug("<0.5 HL = %d\r\n", (int)pwmPercent);
        }
        pwmPercent += (man.fSetupTemper - 30.0)*2;
    }    
#endif
    //cDebug("HL = %d\r\n", (int)pwmPercent);
	
	if(pwmPercent >= 100)
		pwmPercent = 100;
	else if(pwmPercent <= 0)
		pwmPercent = 0;
	
	HeatLine_Adjust(pwmPercent);
#endif
}

void InnerOxygenProcess(void)
{
    uint8_t i;
    float vol, result;

    //内部氧浓度
    Sort16(man.innerOxygenBufferADC, OXYGENBUFFER_SIZE);
    man.innerOxygenADC = GetAverage16(man.innerOxygenBufferADC, OXYGENBUFFER_SIZE, OXYGENBUFFER_SIZE/4);       
    if(man.innerOxygenADC <= 0x1000)
    {
        cDebug("CHIP0 RESET=============================\r\n");
        TM770X_Init(CHIP0_TM7706);
        Delay10ms();    
    }
    
    if(man.innerOxygenADC > 0x8000)
        vol = (float)(man.innerOxygenADC-0x8000)/(float)(0x7FFF)*OXYGEN_ADC_INPUT_VOLTAGE_RANGE_MV;  //14.2mV
    else
        vol = 0;
    for(i=0;i<SIZEOF(OxygenSensor_Table);i++)
    {
        if(vol < OxygenSensor_Table[i].voltageMV)
            break;
    }
    if(i>0) i--;
    result = OxygenSensor_K[i] * (vol - OxygenSensor_Table[i].voltageMV) + OxygenSensor_Table[i].oxygenPercent;
    man.fCurInnerOxygen = (result + man.innerOxygenCalibOffset) * man.innerOxygenCalibRatio; //校准

    if(man.curPage == PAGE_CALIBRATION || man.curPage == PAGE_ADJUSTED)
    {
        char s[10];
        sprintf(s, "%f", man.fCurInnerOxygen);
        //cDebug("the man.curInnerOxygen = %s\r\n", s);
        //cDebug("strlen(s) = %d\r\n", (int)strlen(s));
        SetTextValueLen(0x0428, s, 6);
    }

    man.curInnerOxygen = man.fCurInnerOxygen;

    if(man.curInnerOxygen < 19)
        man.curInnerOxygen = 0;
    else if(man.curInnerOxygen >= 18 && man.curInnerOxygen < 21)
        man.curInnerOxygen = 21;
    if(man.curInnerOxygen > 99) man.curInnerOxygen = 99;
    Int2String(man.curInnerOxygen, man.sCurOxygen);//更新氧浓度显示
	SetTextValueLen(VAR_ADDR_CURRENTOXYGEN, man.sCurOxygen, 2);       
    //cDebug("\r\nThe inner oxygen ADC is 0x%04x\r\n", (int)man.innerOxygenADC);
}

void MaskOxygenProcess(void)
{
    uint8_t i;
    float vol, result;

    //氧罩氧浓度
    Sort16(man.maskOxygenBufferADC, OXYGENBUFFER_SIZE);
    man.maskOxygenADC = GetAverage16(man.maskOxygenBufferADC, OXYGENBUFFER_SIZE, OXYGENBUFFER_SIZE/4);
//        if(man.maskOxygenADC <= 0x1000)
//        {
//            cDebug("=============================\r\n");
//            TM770X_Init(CHIP0_TM7706);
//            //Delay10ms();    
//        }

    if(man.maskOxygenADC > 0x8000)
        vol = (float)(man.maskOxygenADC-0x8000)/(float)(0x7FFF)*OXYGEN_ADC_INPUT_VOLTAGE_RANGE_MV;  //14.2mV
    else
        vol = 0;
    for(i=0;i<SIZEOF(OxygenSensor_Table);i++)
    {
        if(vol < OxygenSensor_Table[i].voltageMV)
            break;
    }
    if(i>0) i--;
    result = OxygenSensor_K[i] * (vol - OxygenSensor_Table[i].voltageMV) + OxygenSensor_Table[i].oxygenPercent;
    man.fCurMaskOxygen = (result + man.maskOxygenCalibOffset) * man.maskOxygenCalibRatio; //校准

    if(man.curPage == PAGE_CALIBRATION || man.curPage == PAGE_ADJUSTED)
    {
        char s[10];
        sprintf(s, "%f", man.fCurMaskOxygen);
        SetTextValueLen(0x042C, s, 6);
    }

    man.curMaskOxygen = man.fCurMaskOxygen;

    if(man.curMaskOxygen < 19)
        man.curMaskOxygen = 0;
    else if(man.curMaskOxygen >= 18 && man.curMaskOxygen < 21)
        man.curMaskOxygen = 21;
    if(man.curMaskOxygen > 99) man.curMaskOxygen = 99;

    if(man.oxygenMode == MODE_OXYGENAIR)
    {
        Int2String(man.curMaskOxygen, man.sCurOxygen);//更新氧浓度显示
    	SetTextValueLen(VAR_ADDR_CURRENTMASKOXYGEN, man.sCurOxygen, 2);
    }       
    //cDebug("The mask oxygen ADC is 0x%04x\r\n", (int)man.maskOxygenADC);
}

void InnerTemperProcess(void)
{
    uint8_t i;
    float vol, res, result;

    
 
 #ifdef HARDWARE_VERSION_18A 
 
   	//内部温度
    Sort32(man.innerTemperBufferADC, TEMPERBUFFER_SIZE);
    man.innerTemperADC = GetAverage32(man.innerTemperBufferADC, TEMPERBUFFER_SIZE, TEMPERBUFFER_SIZE/4);
			   
	if(man.innerTemperADC <= 0x100000)
    {
        cDebug("CHIP1 RESET=============================\r\n");
        TM770X_Init(CHIP1_TM7706);
        Delay10ms();
    } 

	//转换成实际温度
    if(man.innerTemperADC > 0x800000)
        vol = ((float)man.innerTemperADC-(float)0x800000)/(float)(0x7FFFFF)*TEMPER_ADC_INPUT_VOLTAGE_RANGE_MV+TEMPER_ADC_NEGATIVE_INPUT_VOLTAGE_MV;  //109.6mV
    else
        vol = 0;
#endif 

#ifdef HARDWARE_VERSION_24A
	//内部温度
    Sort16(man.innerTemperBufferADC, TEMPERBUFFER_SIZE);
    man.innerTemperADC = GetAverage16(man.innerTemperBufferADC, TEMPERBUFFER_SIZE, TEMPERBUFFER_SIZE/4);
	   
	//转换成实际温度
    if(man.innerTemperADC > 0x8000)
        vol = ((float)man.innerTemperADC-(float)0x8000)/(float)(0x7FFF)*TEMPER_ADC_INPUT_VOLTAGE_RANGE_MV+TEMPER_ADC_NEGATIVE_INPUT_VOLTAGE_MV;  //109.6mV
    else
        vol = 0;
#endif 

    res = TEMPER_ADC_PULLUP_RESISTOR_OHM*vol/(TEMPER_ADC_POWER_VOLTAGE_MV-vol);  //111.9ohm
    for(i=0;i<SIZEOF(Pt100_Table);i++)
    {
        if(res < Pt100_Table[i].resistorKOhm)
            break;
    }
    if(i>0) i--;
    result = Pt100_K[i] * (res - Pt100_Table[i].resistorKOhm) + Pt100_Table[i].temperature;
    man.curInnerTemper = (result + man.innerTemperCalibOffset) * man.innerTemperCalibRatio; //校准

    if(man.curPage == PAGE_CALIBRATION || man.curPage == PAGE_ADJUSTED)
    {
        char s[10];
        sprintf(s, "%f", man.curInnerTemper);
        SetTextValueLen(0x0430, s, 6);
    }

    //cDebug("The inner temper ADC is 0x%04x\r\n", (int)man.innerTemperADC);
}

void CubeTemperProcess(void)
{
    uint8_t i;
    float vol, res, result;

    

#ifdef HARDWARE_VERSION_18A
	//管道温度
    Sort32(man.cubeTemperBufferADC, TEMPERBUFFER_SIZE);
    man.cubeTemperADC = GetAverage32(man.cubeTemperBufferADC, TEMPERBUFFER_SIZE, TEMPERBUFFER_SIZE/4);

    if(man.cubeTemperADC <= 0x100000)
    {
        cDebug("CHIP2 RESET=============================\r\n");
        TM770X_Init(CHIP2_TM7707);
        Delay10ms();
    }

	//转换成实际温度
    if(man.cubeTemperADC > 0x800000)
        vol = ((float)man.cubeTemperADC-(float)0x800000)/(float)(0x7FFFFF)*TEMPER_ADC_INPUT_VOLTAGE_RANGE_MV+TEMPER_ADC_NEGATIVE_INPUT_VOLTAGE_MV;  //109.6mV
    else
        vol = 0;
#endif

#ifdef HARDWARE_VERSION_24A
	//管道温度
    Sort16(man.cubeTemperBufferADC, TEMPERBUFFER_SIZE);
    man.cubeTemperADC = GetAverage16(man.cubeTemperBufferADC, TEMPERBUFFER_SIZE, TEMPERBUFFER_SIZE/4);

	if(man.cubeTemperADC <= 0x1000)
    {
        cDebug("CHIP1 RESET=============================\r\n");
        TM770X_Init(CHIP1_TM7706);
        Delay10ms();   
    }

    //转换成实际温度
    if(man.cubeTemperADC > 0x8000)
        vol = ((float)man.cubeTemperADC-(float)0x8000)/(float)(0x7FFF)*TEMPER_ADC_INPUT_VOLTAGE_RANGE_MV+TEMPER_ADC_NEGATIVE_INPUT_VOLTAGE_MV;  //109.6mV
    else
        vol = 0;
#endif 

    res = TEMPER_ADC_PULLUP_RESISTOR_OHM*vol/(TEMPER_ADC_POWER_VOLTAGE_MV-vol);  //111.9ohm
    for(i=0;i<SIZEOF(Pt100_Table);i++)
    {
        if(res < Pt100_Table[i].resistorKOhm)
            break;
    }
    if(i>0) i--;
    result = Pt100_K[i] * (res - Pt100_Table[i].resistorKOhm) + Pt100_Table[i].temperature;
    man.curCubeTemper = (result + man.cubeTemperCalibOffset) * man.cubeTemperCalibRatio; //校准
    
    if(man.curPage == PAGE_CALIBRATION || man.curPage == PAGE_ADJUSTED)
    {
        char s[10];
        sprintf(s, "%f", man.curCubeTemper);
        SetTextValueLen(0x0434, s, 6);
    }
    
    sprintf(man.stringTemp, "%f", man.curCubeTemper);
    SetTextValueLen(VAR_ADDR_CURRENTTEMPER, man.stringTemp, 4);

    //cDebug("The cube temper ADC is 0x%04x\r\n", (int)man.cubeTemperADC);
}

void MaskTemperProcess(void)
{
    uint8_t i;
    float vol, res, result;

    

#ifdef HARDWARE_VERSION_18A
	//氧罩温度
    Sort32(man.maskTemperBufferADC, TEMPERBUFFER_SIZE);
    man.maskTemperADC = GetAverage32(man.maskTemperBufferADC, TEMPERBUFFER_SIZE, TEMPERBUFFER_SIZE/4);

    //转换成实际温度
    if(man.maskTemperADC > 0x800000)     
        vol = ((float)man.maskTemperADC-(float)0x800000)/(float)(0x7FFFFF)*TEMPER_ADC_INPUT_VOLTAGE_RANGE_MV+TEMPER_ADC_NEGATIVE_INPUT_VOLTAGE_MV;  //109.6mV
    else
        vol = 0;
#endif

#ifdef HARDWARE_VERSION_24A
	//氧罩温度
    Sort16(man.maskTemperBufferADC, TEMPERBUFFER_SIZE);
    man.maskTemperADC = GetAverage16(man.maskTemperBufferADC, TEMPERBUFFER_SIZE, TEMPERBUFFER_SIZE/4);

    //转换成实际温度
    if(man.maskTemperADC > 0x8000)     
        vol = ((float)man.maskTemperADC-(float)0x8000)/(float)(0x7FFF)*TEMPER_ADC_INPUT_VOLTAGE_RANGE_MV+TEMPER_ADC_NEGATIVE_INPUT_VOLTAGE_MV;  //109.6mV
    else
        vol = 0;
#endif 

    res = TEMPER_ADC_PULLUP_RESISTOR_OHM*vol/(TEMPER_ADC_POWER_VOLTAGE_MV-vol);  //111.9ohm
    //res = (TEMPER_ADC_PULLUP_RESISTOR_OHM+man.maskTemperCalibOffset)*vol/(TEMPER_ADC_POWER_VOLTAGE_MV-vol);  //111.9ohm
    for(i=0;i<SIZEOF(Pt100_Table);i++)
    {
        if(res < Pt100_Table[i].resistorKOhm)
            break;
    }
    if(i>0) i--;
    result = Pt100_K[i] * (res - Pt100_Table[i].resistorKOhm) + Pt100_Table[i].temperature;
    man.curMaskTemper = (result + man.maskTemperCalibOffset) * man.maskTemperCalibRatio; //校准       
    //man.curMaskTemper = result;

    if(man.curPage == PAGE_CALIBRATION || man.curPage == PAGE_ADJUSTED)
    {
        char s[10];
        sprintf(s, "%f", man.curMaskTemper);
        SetTextValueLen(0x0438, s, 6);
    }

    if(man.oxygenMode == MODE_OXYGENAIR)
    {
        //sprintf(man.stringTemp, "%f", man.curInnerTemper); //暂时用于温度PID测试
        sprintf(man.stringTemp, "%f", man.curMaskTemper);
        SetTextValueLen(VAR_ADDR_CURRENTMASKTEMPER, man.stringTemp, 4);
    }
    //cDebug("The mask temper ADC is 0x%04x\r\n", (int)man.maskTemperADC);
}

#ifdef HARDWARE_VERSION_18A
void ADCGet(void)
{
    uint8_t i;
    uint8_t readData2[2];
	uint8_t readData4[4];
    static uint8_t flag = 0;

    if(man.oxygenMode == MODE_OXYGENAIR)//空氧模式需要采集2氧浓度传感器和3温度传感器数据
    {
        if(flag == 0)
            flag = 1;
        else if(flag == 1)
            flag = 2;
        else
            flag = 0;
    }
    else  //其他模式不用采集罩氧和罩温
    {
        if(flag == 0)
            flag = 2;
        else
            flag = 0;
    }

    //flag = 2;

    if(flag == 0)
    {
        TM770X_ReadData(0, readData2);
        TM770X_ReadData(3, readData4);
        os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);

        for(i=0;i<OXYGENBUFFER_SIZE;i++)
        {
            //内部氧浓度
            readData2[0] = 0;
            readData2[1] = 0;               
            TM770X_ReadData(0, readData2);
            man.innerOxygenBufferADC[i] = *((uint16_t*)readData2);

            //管道温度
            readData4[0] = 0;
            readData4[1] = 0;
			readData4[2] = 0;
			readData4[3] = 0;              
            TM770X_ReadData(3, readData4);
            man.cubeTemperBufferADC[i] = (*((uint32_t*)readData4))>>8;

            os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);
        }

        //========================================================================================================
        InnerOxygenProcess();       
       
        //========================================================================================================    
        CubeTemperProcess();
    }
    else if(flag == 1)
    {
        TM770X_ReadData(1, readData2);
        TM770X_ReadData(4, readData4);
        os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);

        for(i=0;i<OXYGENBUFFER_SIZE;i++)
        {
            //氧罩氧浓度
            readData2[0] = 0;
            readData2[1] = 0;               
            TM770X_ReadData(1, readData2);
            man.maskOxygenBufferADC[i] = *((uint16_t*)readData2);

            //氧罩温度
            readData4[0] = 0;
            readData4[1] = 0;
			readData4[2] = 0;
            readData4[3] = 0;                
            TM770X_ReadData(4, readData4);
            man.maskTemperBufferADC[i] = (*((uint32_t*)readData4))>>8;

            os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);
        }

        //========================================================================================================
        MaskOxygenProcess();

        //========================================================================================================
        MaskTemperProcess();
    }
    else
    {
        TM770X_ReadData(2, readData4);
        os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);

        for(i=0;i<OXYGENBUFFER_SIZE;i++)
        {
            //内部温度
            readData4[0] = 0;
            readData4[1] = 0;
			readData4[2] = 0;
            readData4[3] = 0;               
            TM770X_ReadData(2, readData4);
            man.innerTemperBufferADC[i] = (*((uint32_t*)readData4))>>8;

            os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);
        }

        //========================================================================================================
        InnerTemperProcess();
    } 
}
#endif


#ifdef HARDWARE_VERSION_24A
void ADCGet(void)
{
    uint8_t i;
    uint8_t readData2[2];

    static uint8_t flag = 0;

    if(man.oxygenMode == MODE_OXYGENAIR)//空氧模式需要采集2氧浓度传感器和3温度传感器数据
    {
        if(flag == 0)
            flag = 1;
        else if(flag == 1)
            flag = 2;
        else
            flag = 0;
    }
    else  //其他模式不用采集罩氧和罩温
    {
        if(flag == 0)
            flag = 2;
        else
            flag = 0;
    }

    //flag = 2;

    if(flag == 0)
    {
        TM770X_ReadData(0, readData2);
        TM770X_ReadData(3, readData2);
        os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);

        for(i=0;i<OXYGENBUFFER_SIZE;i++)
        {
            //内部氧浓度
            readData2[0] = 0;
            readData2[1] = 0;               
            TM770X_ReadData(0, readData2);
            man.innerOxygenBufferADC[i] = *((uint16_t*)readData2);

            //管道温度
            readData2[0] = 0;
            readData2[1] = 0;              
            TM770X_ReadData(3, readData2);
            man.cubeTemperBufferADC[i] = *((uint16_t*)readData2);

            os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);
        }

        //========================================================================================================
        InnerOxygenProcess();       
       
        //========================================================================================================    
        CubeTemperProcess();
    }
    else if(flag == 1)
    {
        TM770X_ReadData(1, readData2);
        TM770X_ReadData(4, readData2);
        os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);

        for(i=0;i<OXYGENBUFFER_SIZE;i++)
        {
            //氧罩氧浓度
            readData2[0] = 0;
            readData2[1] = 0;               
            TM770X_ReadData(1, readData2);
            man.maskOxygenBufferADC[i] = *((uint16_t*)readData2);

            //氧罩温度
            readData2[0] = 0;
            readData2[1] = 0;               
            TM770X_ReadData(4, readData2);
            man.maskTemperBufferADC[i] = *((uint16_t*)readData2);

            os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);
        }

        //========================================================================================================
        MaskOxygenProcess();

        //========================================================================================================
        MaskTemperProcess();
    }
    else
    {
        TM770X_ReadData(2, readData2);
        os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);
//        TM770X_ReadData(2, readData2);
//        os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);

        for(i=0;i<OXYGENBUFFER_SIZE;i++)
        {
            //内部温度
            readData2[0] = 0;
            readData2[1] = 0;               
            TM770X_ReadData(2, readData2);
            man.innerTemperBufferADC[i] = *((uint16_t*)readData2);

            os_wait(K_TMO, OS_TICK/TM7706_OUTPUT_HZ, 0);
        }

        //========================================================================================================
        InnerTemperProcess();
    } 
}
#endif

void AlarmCheck(void)
{
    static uint8_t oxycureStateLast = 0;
    static uint8_t oxycureState = 0;

    //过流保护
    if(I_OVER == 0)
    {
        man.startFlag = 0;
        stepMotor_Stop();//停止步进电机
        HeatPlate_Adjust(0);//停止加热盘加热
        HeatLine_Adjust(0); //停止加热线加热
    }

    man.alarmStatusLast = man.alarmStatus;

    //温度超过43℃检测、保护和报警
    if(man.oxygenMode == MODE_OXYGENAIR)
    {
        if((man.curInnerTemper > TEMPER_MAX && man.curInnerTemper < UNCONNECT_TEMPER_MAX)
             || (man.curCubeTemper > TEMPER_MAX && man.curCubeTemper < UNCONNECT_TEMPER_MAX)
             || (man.curMaskTemper > TEMPER_MAX && man.curMaskTemper < UNCONNECT_TEMPER_MAX))
            man.alarmStatus |= ALARM_TEMPER;
        else
            man.alarmStatus &= ~ALARM_TEMPER;    
    }
    else// if(man.oxygenMode == MODE_ANABIOSIS || man.oxygenMode == MODE_CPAP)
    {
        if((man.curInnerTemper > TEMPER_MAX && man.curInnerTemper < UNCONNECT_TEMPER_MAX)
             || (man.curCubeTemper > TEMPER_MAX && man.curCubeTemper < UNCONNECT_TEMPER_MAX))
            man.alarmStatus |= ALARM_TEMPER;
        else
            man.alarmStatus &= ~ALARM_TEMPER;
    } 

    //流量检测
    //if(MIN1 == 0)//测试屏蔽
    if(MIN1 == 1) //实际工作时使用
        man.alarmStatus |= ALARM_FLUX;
    else
        man.alarmStatus &= ~ALARM_FLUX;

    //连接检测
    if(man.oxygenMode == MODE_ANABIOSIS || man.oxygenMode == MODE_CPAP)
    {
        if(man.curInnerTemper > UNCONNECT_TEMPER_MAX || man.curCubeTemper > UNCONNECT_TEMPER_MAX //温度接近79℃证明没连接传感器
            || Abs(man.curInnerTemper - man.curCubeTemper) > TEMPER_DIFFERENCE) //温度差太大，证明有一个温度传感器脱落
            man.alarmStatus |= ALARM_CONNECT;
        else
            man.alarmStatus &= ~ALARM_CONNECT;
    }
    else if(man.oxygenMode == MODE_OXYGENAIR)
    {
        if(man.curInnerTemper > UNCONNECT_TEMPER_MAX || man.curCubeTemper > UNCONNECT_TEMPER_MAX || man.curMaskTemper > UNCONNECT_TEMPER_MAX
            || Abs(man.curInnerTemper - man.curCubeTemper) > TEMPER_DIFFERENCE //温度差太大，证明有一个温度传感器脱落
            || Abs(man.curMaskTemper - man.curCubeTemper) > TEMPER_DIFFERENCE
            || Abs(man.curInnerTemper - man.curMaskTemper) > TEMPER_DIFFERENCE)
            man.alarmStatus |= ALARM_CONNECT;
        else
            man.alarmStatus &= ~ALARM_CONNECT;    
    }

    //氧疗异常检测
    //if(man.startFlag) //运行才检测
    {
//        if(man.oxygenMode == MODE_ANABIOSIS || man.oxygenMode == MODE_CPAP)//复苏模式和CPAP模式
//        {
//            if(man.curInnerOxygen > man.iHighOxygen || man.curInnerOxygen < man.iLowOxygen)
//                man.alarmStatus |= ALARM_OXYCURE_OXYGEN;
//            else
//                man.alarmStatus &= ~ALARM_OXYCURE_OXYGEN;
//                    
//            if(man.curInnerTemper > man.fHighTemper || man.curInnerTemper < man.fLowTemper
//                || man.curCubeTemper > man.fHighTemper || man.curCubeTemper < man.fLowTemper)
//                man.alarmStatus |= ALARM_OXYCURE_TEMPER;
//            else
//                man.alarmStatus &= ~ALARM_OXYCURE_TEMPER;
//        }
//        else if(man.oxygenMode == MODE_OXYGENAIR)//空氧模式
        {
//            if(man.curInnerOxygen > man.iHighOxygen || man.curInnerOxygen < man.iLowOxygen
//                || man.curMaskOxygen > man.iHighOxygen || man.curMaskOxygen < man.iLowOxygen)
            if(man.curInnerOxygen > man.iHighOxygen || man.curInnerOxygen < man.iLowOxygen)
            {
                if(man.startFlag)
                    man.alarmStatus |= ALARM_OXYCURE_OXYGEN;
            }
            else
                man.alarmStatus &= ~ALARM_OXYCURE_OXYGEN;

//            if(man.curInnerTemper > man.fHighTemper || man.curInnerTemper < man.fLowTemper
//                || man.curCubeTemper > man.fHighTemper || man.curCubeTemper < man.fLowTemper            
//                || man.curMaskTemper > man.fHighTemper || man.curMaskTemper < man.fLowTemper)
            if(man.curCubeTemper > man.fHighTemper || man.curCubeTemper < man.fLowTemper)
            {
                if(man.startFlag)
                    man.alarmStatus |= ALARM_OXYCURE_TEMPER;
            }
            else
                man.alarmStatus &= ~ALARM_OXYCURE_TEMPER;    
        }
    }

    man.alarmStatusRaisingEdge = (man.alarmStatusLast ^ man.alarmStatus) & man.alarmStatus;
    man.alarmStatusFallingEdge = (man.alarmStatusLast ^ man.alarmStatus) & man.alarmStatusLast;

    oxycureStateLast = oxycureState;
    oxycureState = !!(man.alarmStatus&ALARM_OXYCURE);

    //==========================================================================================
    //流量报警
    if(man.alarmStatusRaisingEdge&ALARM_FLUX)
    {
        SetTextValueLen(VAR_ADDR_WARNINGFLUX, FLUX_WARNING_STRING, 8);
        SetTextFontColor(VAR_ADDR_WARNINGFLUX_PTR, COLOR_RED);
    }
    else if(man.alarmStatusFallingEdge&ALARM_FLUX)
    {
        SetTextValueLen(VAR_ADDR_WARNINGFLUX, FLUX_WORKING_STRING, 8);
        SetTextFontColor(VAR_ADDR_WARNINGFLUX_PTR, COLOR_BLACK);
    }

    //连接报警
    if(man.alarmStatusRaisingEdge&ALARM_CONNECT)
    {
        SetTextValueLen(VAR_ADDR_WARNINGCONNECT, CONNECT_WARNING_STRING, 8);
        SetTextFontColor(VAR_ADDR_WARNINGCONNECT_PTR, COLOR_RED);
    }
    else if(man.alarmStatusFallingEdge&ALARM_CONNECT)
    {
        SetTextValueLen(VAR_ADDR_WARNINGCONNECT, CONNECT_WORKING_STRING, 8);
        SetTextFontColor(VAR_ADDR_WARNINGCONNECT_PTR, COLOR_BLACK);
    }

#if 1
    //氧疗异常报警
    if(man.alarmStatusRaisingEdge&ALARM_OXYCURE)
    {
        if(!(man.silenceStatus&0x80))
        {
            SetTextValueLen(VAR_ADDR_WARNINGWORKING, OXYCURE_WARNING_STRING, 8); //氧疗异常
            SetTextFontColor(VAR_ADDR_WARNINGWORKING_PTR, COLOR_RED);
        }

        if(man.alarmStatus&ALARM_OXYCURE_OXYGEN)
            SetTextFontColor(VAR_ADDR_CURRENTOXYGEN_PTR, COLOR_RED);
        if(man.alarmStatus&ALARM_OXYCURE_TEMPER)
            SetTextFontColor(VAR_ADDR_CURRENTTEMPER_PTR, COLOR_RED);
    }

    if(man.alarmStatusFallingEdge&ALARM_OXYCURE_OXYGEN)
        SetTextFontColor(VAR_ADDR_CURRENTOXYGEN_PTR, COLOR_GREEN);
    if(man.alarmStatusFallingEdge&ALARM_OXYCURE_TEMPER)
        SetTextFontColor(VAR_ADDR_CURRENTTEMPER_PTR, COLOR_GREEN);
//    if(!(man.alarmStatusFallingEdge&ALARM_OXYCURE))
//    {
//        SetTextValueLen(VAR_ADDR_WARNINGWORKING, OXYCURE_WORKING_STRING, 8); //氧疗正常
//        SetTextFontColor(VAR_ADDR_WARNINGWORKING_PTR, COLOR_BLACK);
//    }
    if((oxycureStateLast^oxycureState)&oxycureStateLast)
    {
        SetTextValueLen(VAR_ADDR_WARNINGWORKING, OXYCURE_WORKING_STRING, 8); //氧疗正常
        SetTextFontColor(VAR_ADDR_WARNINGWORKING_PTR, COLOR_BLACK);
    }
//    if(!(man.alarmStatus&ALARM_OXYCURE_OXYGEN))
//        SetTextFontColor(VAR_ADDR_CURRENTOXYGEN_PTR, COLOR_GREEN);
//    if(!(man.alarmStatusFallingEdge&ALARM_OXYCURE_TEMPER))
//        SetTextFontColor(VAR_ADDR_CURRENTTEMPER_PTR, COLOR_GREEN);
//    if(!(man.alarmStatusFallingEdge&ALARM_OXYCURE == ALARM_OXYCURE))
//    {
//        SetTextValueLen(VAR_ADDR_WARNINGWORKING, OXYCURE_WORKING_STRING, 8);
//        SetTextFontColor(VAR_ADDR_WARNINGWORKING_PTR, COLOR_BLACK);
//    }
#endif

    //温度超过43℃报警
    if(man.alarmStatusRaisingEdge&ALARM_TEMPER)
    {
        man.startFlag = 0;
        stepMotor_Stop();//停止步进电机
        HeatPlate_Adjust(0);//停止加热盘加热
        HeatLine_Adjust(0); //停止加热线加热

        man.curPage = PAGE_OVERTEMPER43;
        SetScreen(man.curPage);

        cDebug("The temperature is over 43℃, please power off then power on!\r\n");    
    }
}

void AutoCalibration(uint8 flag)
{
    uint8_t i;

    switch(man.autoCalibState)
    {
        case 0: //先将混合器调到中间
            //cDebug("Calibration\r\n");
            if(flag)
                man.fSetupOxygen = 60.0;
            else
                man.fSetupOxygen = 40.0;

            OxygenPercentControl();

//            man.userTimeCnt = 2*60; //2分钟倒数
//            man.userTimeOutFlag = 1;

            if(Abs(man.fSetupOxygen - man.fCurInnerOxygen) < 5) //0.5
            {
                man.autoCalibState = 1;
                man.beeperTimes = 1;
            }
        break;
        case 1:  //提示拔掉氧气
            man.curPage = PAGE_UNPLUGOXYGEN; 
            SetScreen(man.curPage);
        break;
        case 2:  //校准21%浓度
            for(i=0;i<20;i++)
                os_wait(K_TMO, 200, 0); //等待12秒，氧浓度稳定在空气浓度

            man.innerOxygenCalibOffset = 0.0;
            man.maskOxygenCalibOffset = 0.0;
            man.innerOxygenCalibRatio = 1.0;
            man.maskOxygenCalibRatio = 1.0;

            ADCGet();

            cDebug("man.fCurInnerOxygen = %f\r\n", man.fCurInnerOxygen);

            //内氧传感器校准
            if(man.fCurInnerOxygen < 21.0 && Abs(man.fCurInnerOxygen - 21.0) > 4.0)
            {
                man.curPage = PAGE_REPLACE_S1; 
                SetScreen(man.curPage);

                man.beeperTimes = 5;

                while(1)
                    os_wait(K_TMO, 10, 0);
            }
            else
            {
                man.innerOxygenCalibOffset -= man.fCurInnerOxygen - 21.0;  
            }

            
//            if(man.oxygenMode == MODE_OXYGENAIR)
//            {
//                cDebug("man.fCurMaskOxygen = %f\r\n", man.fCurMaskOxygen);
//
//                //校准氧罩氧浓度传感器
//                if(man.fCurMaskOxygen < 21.0 && Abs(man.fCurMaskOxygen - 21.0) > 4.0)
//                {
//                    man.curPage = PAGE_REPLACE_S2; 
//                    SetScreen(man.curPage);
//    
//                    man.beeperTimes = 5;
//    
//                    while(1)
//                        os_wait(K_TMO, 10, 0);
//                }
//                else
//                {
//                    man.maskOxygenCalibOffset -= man.fCurMaskOxygen - 21.0;  
//                }
//            }
            
            if(flag)
                man.autoCalibState = 3;
            else
                man.autoCalibState = 7;  //去掉99%的末端校准
            
            man.beeperTimes = 1;
                
        break;
        case 3:
            man.curPage = PAGE_UNPLUGAIR; 
            SetScreen(man.curPage);
        break;
        case 4:
            for(i=0;i<12;i++)
                os_wait(K_TMO, 200, 0); //等待12秒，氧浓度稳定在空气浓度

            ADCGet();
  
            //校准内部氧浓度传感器
            //man.innerOxygenCalibRatio *= (99.5 - man.innerOxygenCalibOffset)/(man.fCurInnerOxygen - man.innerOxygenCalibOffset);
            man.innerOxygenCalibRatio *= 99.5/man.fCurInnerOxygen;

            if(man.oxygenMode == MODE_OXYGENAIR)
            {
                //校准氧罩氧浓度传感器
                //man.maskOxygenCalibRatio *= (99.5 - man.maskOxygenCalibOffset)/(man.fCurMaskOxygen - man.maskOxygenCalibOffset);
                man.maskOxygenCalibRatio *= 99.5/man.fCurMaskOxygen;
            }

            man.autoCalibState = 7; //不必再校准21%氧浓度

            man.beeperTimes = 1;
        break;
        case 5:  //提示拔掉氧气 
            man.curPage = PAGE_UNPLUGOXYGEN; 
            SetScreen(man.curPage);
        break;
        case 6:
            for(i=0;i<12;i++)
                os_wait(K_TMO, 200, 0); //等待12秒，氧浓度稳定在空气浓度

            ADCGet();

            //内氧传感器校准
            if(man.fCurInnerOxygen < 21.0 && Abs(man.fCurInnerOxygen - 21.0) > 4.0)
            {
                man.curPage = PAGE_REPLACE_S1; 
                SetScreen(man.curPage);

                man.beeperTimes = 5;

                while(1)
                    os_wait(K_TMO, 10, 0);
            }
            else
            {
                man.innerOxygenCalibOffset -= man.fCurInnerOxygen - 21.0;  
            }

            
//            if(man.oxygenMode == MODE_OXYGENAIR)
//            {
//                //校准氧罩氧浓度传感器
//                if(man.fCurMaskOxygen < 21.0 && Abs(man.fCurMaskOxygen - 21.0) > 4.0)
//                {
//                    man.curPage = PAGE_REPLACE_S2; 
//                    SetScreen(man.curPage);
//    
//                    man.beeperTimes = 5;
//    
//                    while(1)
//                        os_wait(K_TMO, 10, 0);
//                }
//                else
//                {
//                    man.maskOxygenCalibOffset -= man.fCurMaskOxygen - 21.0;  
//                }
//            }

            man.autoCalibState = 7;
        break;
        case 7:
            man.autoCalibState = 0;
            man.autoCalibFlag = 0;

//            cDebug("innerOxygenCalibOffset = %f\r\n", man.innerOxygenCalibOffset);
//            cDebug("maskOxygenCalibOffset = %f\r\n", man.maskOxygenCalibOffset);
//            cDebug("innerOxygenCalibRatio = %f\r\n", man.innerOxygenCalibRatio);
//            cDebug("maskOxygenCalibRatio = %f\r\n", man.maskOxygenCalibRatio);

            //保存校准参数
            IapErase(EEPROM_BASEADDR_CALIB);
            os_wait(K_TMO, 2, 0); //擦除需要等待4~6ms，这里等待10ms
            EEPROM_WriteBytes(EEPROM_BASEADDR_CALIB, (uint8_t*)(man.pCalib), sizeof(float)*10);

            //读取RTC时间
            GetRTC();
            while(!(man.autoCalibFlag&0x80))
                os_wait(K_TMO, 10, 0);
            if(man.autoCalibFlag&0x80)
            {
                EEPROM_WriteBytes(EEPROM_BASEADDR_AUTOCALIB, (uint8_t*)(&man.autoCalibDate), 4);
                cDebug("man.autoCalibDate = %ld\r\n", man.autoCalibDate);
            }
            man.curPage = PAGE_CALIBCOMPLETE; 
            SetScreen(man.curPage);
            
            man.beeperTimes = 2;           
        break;
        default:
        break;
    }

}
