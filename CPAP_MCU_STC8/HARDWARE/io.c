#include "io.h"
#include "uart.h"
#include "misc.h"
#include "timer.h"
#include "tm770x.h"
#include "stepmotor.h"
#include "eeprom.h"

#include "misc.h"
#include "management.h"

#include "hmi_driver.h"

//const char OXYGEN_MODE_STRING[] = {0xD1, 0xF5, 0xD5, 0xD6, 0xC4, 0xA3, 0xCA, 0xBD}; //氧罩模式
//const char RHINOBYON_MODE_STRING[] = {0xB1, 0xC7, 0xC8, 0xFB, 0xC4, 0xA3, 0xCA, 0xBD}; //鼻塞模式
const char OXYGEN_MODE_STRING[] = {0xBF, 0xD5, 0xD1, 0xF5, 0xC4, 0xA3, 0xCA, 0xBD}; //空氧模式
const char RHINOBYON_MODE_STRING[] = {0xB8, 0xB4, 0xCB, 0xD5, 0xC4, 0xA3, 0xCA, 0xBD}; //复苏模式
const char CAPA_MODE_STRING[] = {0x43, 0x50, 0x41, 0x50, 0xC4, 0xA3, 0xCA, 0xBD}; //CPAP模式

const char FLUX_WARNING_STRING[] = {0xC1, 0xF7, 0xC1, 0xBF, 0xD2, 0xEC, 0xB3, 0xA3}; //流量异常
const char FLUX_WORKING_STRING[] = {0xC1, 0xF7, 0xC1, 0xBF, 0xD5, 0xFD, 0xB3, 0xA3}; //流量正常

const char CONNECT_WARNING_STRING[] = {0xC1, 0xAC, 0xBD, 0xD3, 0xD2, 0xEC, 0xB3, 0xA3}; //连接异常
const char CONNECT_WORKING_STRING[] = {0xC1, 0xAC, 0xBD, 0xD3, 0xD5, 0xFD, 0xB3, 0xA3}; //连接正常

const char OXYCURE_WARNING_STRING[] = {0xD1, 0xF5, 0xC1, 0xC6, 0xD2, 0xEC, 0xB3, 0xA3}; //氧疗异常
const char OXYCURE_WORKING_STRING[] = {0xD1, 0xF5, 0xC1, 0xC6, 0xD5, 0xFD, 0xB3, 0xA3}; //氧疗正常
const char TEMPER_WARNING_STRING[] = {0xCE, 0xC2, 0xB6, 0xC8, 0xD2, 0xEC, 0xB3, 0xA3}; //温度异常
const char SILENCE_STATUS_STRING[] = {0xBE, 0xB2, 0xD2, 0xF4, 0xD7, 0xB4, 0xCC, 0xAC}; //静音状态

const char TILDE_STRING[] = {0x7E, 0x7E, 0x2E, 0x7E}; //波浪符号

//温度超过43℃，请重新上电！
//const int16_t TEMPER_ALARM_STRING[] = {0xCEC2, 0xB6C8, 0xB3AC, 0xB9FD, 0x3433, 0xA1E6, 0xA3AC, 0xC7EB, 0xD6D8, 0xD0C2, 0xC9CF, 0xB5E7};
const char TEMPER_ALARM_STRING[] = {0xCE, 0xC2, 0xB6, 0xC8, 0xB3, 0xAC, 0xB9, 0xFD, 0x34, 0x33, 0xA1, 0xE6, 0xA3, 0xAC, 0xC7, 0xEB, 0xD6, 0xD8, 0xD0, 0xC2, 0xC9, 0xCF, 0xB5, 0xE7};

Key_TypeDef key;

//IO初始化
void IO_Init(void)
{
	//IO--------------------------------------------------------------------------------------
	P0M1 = 0x00;    //双双双双双双双双
	P0M0 = 0xA0;
	P0 = 0xFF;    //M-DiscHeat,SW7,M-DiscLine,SW5,SW3,SW1,SW2,SW6
	//----------------------
	P1M1 = 0x00;    //双双双双双双双双           //LCD_DOUT改成推挽
	P1M0 = 0x01;    //
	P1 = 0xFF;    //XTAL2,XTAL1,A-SCLK,A-DATA,A-DRDY3,未定义,LCD_DIN,LCD_DOUT
	//----------------------
	P2M1 = 0x70;    //双漏漏漏双双双双
	P2M0 = 0x70;
	P2 = 0xFF;    //IIC_WP,LEDM,IIC_SCL,IIC_SDA,RES1,RES2,RES3,LCD_BUSY
	//----------------------
	P3M1 = 0x00;    //双双双推双双双双
	P3M0 = 0x10;
	P3 = 0xEF;    //I-OVER,STM-DIR,STM-PULSE,MBEEP,MIN2,MIN1,M-TXD,M-RXD
	//----------------------
	P4M1 = 0x00;    //双双双双双双双双双
	P4M0 = 0x00;
	P4 = 0xFF;    //A-CS3,A-DRDY1,A-DRDY2,A-RST,SW4,RES4,STM-EN,A-CS1
	
	P5M1 = 0x00;    //双双双双双双双双双
	P5M0 = 0x00;
	P5 = 0xFF;    //---,---,A-CS2,M-CLKA,---,---,---,---

	//SFR---------------------------------------------------------------------------
	WDT_CONTR = 0x00;         //WDT未启动
	PCON = 0x00; //bit7=0 串口波特率不加倍
	//SCON = 0x50;  MODE 1	可变波特率8位数据方式
	//TMOD = 0x00; //T0,MODE0,16位自动重装,T1 MODE1,16位自动重装
	//AUXR = 0xD5; //T0/T1/T2不分频,12T快速模式;T2为串口1的波特率发生器

	//TCON = 0x00; //
	
//
//	AUXR1 = 0x20;	//使用DPTR0,CCP在P24P25P26,串口1在P30/P31
//
//	P_SW2 = 0x00; //
//	ADC_CONTR = 0x00; //AD未启动
//	P4SW = 0x70; //P46P45P44设为IO口
//	WAKE_CLKO = 0x00; //不输出时钟
////	CLK_DIV = 0x00; //时钟不分频,不输出主时钟
//	IE = 0x00; //
//	IE2 = 0x00; //
	IP0 = 0x10; //---PCA,LVD,ADC,UART,T1,EX1,T0,EX0,UART高优先级

    //-------------------------------------------------------------
	//初始化IO
    //TM760X
	A_RST = 1;
    A_CS1 = 1;
    A_CS2 = 1;
    A_CS3 = 1;
    A_SCLK = 1;
    A_DRDY1 = 1;
    A_DRDY2 = 1;
    A_DRDY3 = 1;

    //关闭加热
//    M_DISCHEAT = 0;
//    M_LINEHEAT = 0;
    M_DISCHEAT = 1;
    M_LINEHEAT = 1;

    //步进电机
	STM_EN = 1;
    STM_DIR = 1;
    STM_PULSE = 1;

    //TM770X时钟输出，
    P_SW2 = 0x80;   
    XOSCCR |= 0xC0; //启动外部晶振
    while(XOSCCR&0x01 == 0x00); //外部晶振就绪
    CKSEL = 0x01;                                //主时钟使用外部晶振
    CKSEL |= 0xA0;                               //主时钟32分频输出到P5.4口
    CLKDIV = 0x00;                             //SYSclk = MCLK/1
    //CLKDIV = 0x0A;                             //SYSclk = MCLK/10   //不能正常工作
    IRC24MCR = 0x00;                            //关闭内部24MHz时钟
    P_SW2 = 0x00;

    man.beeperAlarm = 0;
}
                         
void Key_Scan(void)
{
	key.lastKey = key.curKey;

	(KEY_SW1 == 0) ? (key.curKey |= 0x01) :	(key.curKey &= ~0x01);
	(KEY_SW2 == 0) ? (key.curKey |= 0x02) :	(key.curKey &= ~0x02);
	(KEY_SW3 == 0) ? (key.curKey |= 0x04) :	(key.curKey &= ~0x04);
	(KEY_SW4 == 0) ? (key.curKey |= 0x08) :	(key.curKey &= ~0x08);
	(KEY_SW5 == 0) ? (key.curKey |= 0x10) :	(key.curKey &= ~0x10);
	(KEY_SW6 == 0) ? (key.curKey |= 0x20) :	(key.curKey &= ~0x20);
	(KEY_SW7 == 0) ? (key.curKey |= 0x40) :	(key.curKey &= ~0x40);

	key.fallingEdge = (key.lastKey ^ key.curKey) & key.lastKey;
	key.rasingEdge = (key.lastKey ^ key.curKey) & key.curKey;
}

void Key_Process_NoCombo(void)
{
    if(key.rasingEdge&SW1_MASK) //氧浓度增加
	{
        if(man.curPage == PAGE_PAUSE)
        {
			man.iSetupOxygen++;
			if(man.iSetupOxygen > SETUP_OXYGEN_MAX) man.iSetupOxygen = SETUP_OXYGEN_MAX;
            man.fSetupOxygen = man.iSetupOxygen;
			Int2String(man.iSetupOxygen, man.sSetupOxygen);
			SetTextValueLen(VAR_ADDR_SETUPOXYGEN, man.sSetupOxygen, 2);

            man.iLowOxygen = man.iSetupOxygen - 5;
            man.iHighOxygen = man.iSetupOxygen + 5;
            Int2String(man.iLowOxygen, man.sSetupOxygen);
			SetTextValueLen(VAR_ADDR_SETUPOXYGEN_LOW, man.sSetupOxygen, 2);
            Int2String(man.iHighOxygen, man.sSetupOxygen);
			SetTextValueLen(VAR_ADDR_SETUPOXYGEN_HIGH, man.sSetupOxygen, 2);
        }
        else if(man.curPage == PAGE_CALIBRATION)
        {
            char s[10];

            if(man.calibIndex%2)//比例
                *(man.pCalib+man.calibIndex) = *(man.pCalib+man.calibIndex) + 0.01;
            else //偏移
                *(man.pCalib+man.calibIndex) = *(man.pCalib+man.calibIndex) + 0.1;

            sprintf(s, "%f", *(man.pCalib+man.calibIndex));
            SetTextValueLen(VAR_ADDR_CALIB_INNEROXYGEN_OFFSET+man.calibIndex*0x0004, s, 6);
        }
        else if(man.curPage == PAGE_ADJUSTED)
        {
            char s[10];
            float *p = &((man.pAdjustPID+man.adjustPIDIndex/3)->Proportion);
            *(p+man.adjustPIDIndex%3) = *(p+man.adjustPIDIndex%3) + 0.1;

            sprintf(s, "%f", *(p+man.adjustPIDIndex%3));
            SetTextValueLen(VAR_ADDR_ADJUST_INNEROXYGEN_P+man.adjustPIDIndex*0x0004, s, 6);
        }
        //else if(man.curPage == PAGE_MODESELECT)
        else if(man.curPage == man.pageModeSelect)
        {
            man.oxygenMode = MODE_ANABIOSIS; //进入复苏模式
            SetTextValueLen(VAR_ADDR_MODE, RHINOBYON_MODE_STRING, 8);
            SetTextValueLen(VAR_ADDR_CURRENTMASKOXYGEN, TILDE_STRING, 2);
            SetTextValueLen(VAR_ADDR_CURRENTMASKTEMPER, TILDE_STRING, 4);
            man.curPage = PAGE_PAUSE;
            SetScreen(man.curPage);    
        }
	}
	else if(key.rasingEdge&SW2_MASK) //氧浓度减少
	{
        if(man.curPage == PAGE_PAUSE)
        {
			man.iSetupOxygen--;
			if(man.iSetupOxygen < SETUP_OXYGEN_MIN) man.iSetupOxygen = SETUP_OXYGEN_MIN;
            man.fSetupOxygen = man.iSetupOxygen;
			Int2String(man.iSetupOxygen, man.sSetupOxygen);
			SetTextValueLen(VAR_ADDR_SETUPOXYGEN, man.sSetupOxygen, 2);

            man.iLowOxygen = man.iSetupOxygen - 5;
            man.iHighOxygen = man.iSetupOxygen + 5;
            Int2String(man.iLowOxygen, man.sSetupOxygen);
			SetTextValueLen(VAR_ADDR_SETUPOXYGEN_LOW, man.sSetupOxygen, 2);
            Int2String(man.iHighOxygen, man.sSetupOxygen);
			SetTextValueLen(VAR_ADDR_SETUPOXYGEN_HIGH, man.sSetupOxygen, 2);
        }
        else if(man.curPage == PAGE_CALIBRATION)
        {
            char s[10];

            if(man.calibIndex%2)//比例
                *(man.pCalib+man.calibIndex) = *(man.pCalib+man.calibIndex) - 0.01;
            else //偏移
                *(man.pCalib+man.calibIndex) = *(man.pCalib+man.calibIndex) - 0.1;

            sprintf(s, "%f", *(man.pCalib+man.calibIndex));
            SetTextValueLen(VAR_ADDR_CALIB_INNEROXYGEN_OFFSET+man.calibIndex*0x0004, s, 6);
        }
        else if(man.curPage == PAGE_ADJUSTED)
        {
            char s[10];
            float *p = &((man.pAdjustPID+man.adjustPIDIndex/3)->Proportion);
            *(p+man.adjustPIDIndex%3) = *(p+man.adjustPIDIndex%3) - 0.1;

            sprintf(s, "%f", *(p+man.adjustPIDIndex%3));
            SetTextValueLen(VAR_ADDR_ADJUST_INNEROXYGEN_P+man.adjustPIDIndex*0x0004, s, 6);
        }
        //else if(man.curPage == PAGE_MODESELECT)
        else if(man.curPage == man.pageModeSelect)
        {
            man.oxygenMode = MODE_OXYGENAIR; //进入空氧模式
            SetTextValueLen(VAR_ADDR_MODE, OXYGEN_MODE_STRING, 8);
            man.curPage = PAGE_PAUSE;
            SetScreen(man.curPage);
            //cDebug("oxygen dec key press\r\n");    
        }
	}
	if(key.rasingEdge&SW3_MASK)	//温度增加
	{
        if(man.curPage == PAGE_PAUSE)
        {
			man.fSetupTemper += 0.1;
			if(man.fSetupTemper > SETUP_TEMPER_MAX) man.fSetupTemper = SETUP_TEMPER_MAX;
            sprintf(man.stringTemp, "%f", man.fSetupTemper);
            SetTextValueLen(VAR_ADDR_SETUPTEMPER, man.stringTemp, 4);

            man.fLowTemper = man.fSetupTemper - 1.0;
            man.fHighTemper = man.fSetupTemper + 1.0;
            sprintf(man.stringTemp, "%f", man.fLowTemper);
            SetTextValueLen(VAR_ADDR_SETUPTEMPER_LOW, man.stringTemp, 4);
            sprintf(man.stringTemp, "%f", man.fHighTemper);
            SetTextValueLen(VAR_ADDR_SETUPTEMPER_HIGH, man.stringTemp, 4);
        }
        else if(man.curPage == PAGE_CALIBRATION)
        {
            SetTextFontColor(VAR_ADDR_CALIB_INNEROXYGEN_OFFSET_PTR+man.calibIndex*0x0010, COLOR_WHITE);
            man.calibIndex++;
            if(man.calibIndex > 9)
                man.calibIndex = 0;
            SetTextFontColor(VAR_ADDR_CALIB_INNEROXYGEN_OFFSET_PTR+man.calibIndex*0x0010, COLOR_RED);
        }
        else if(man.curPage == PAGE_ADJUSTED)
        {
            SetTextFontColor(VAR_ADDR_ADJUST_INNEROXYGEN_P_PTR+man.adjustPIDIndex*0x0010, COLOR_WHITE);
            man.adjustPIDIndex++;
            if(man.adjustPIDIndex > 8)
                man.adjustPIDIndex = 0;
            SetTextFontColor(VAR_ADDR_ADJUST_INNEROXYGEN_P_PTR+man.adjustPIDIndex*0x0010, COLOR_RED);
        }
	}
	else if(key.rasingEdge&SW4_MASK) //温度减少
	{
        if(man.curPage == PAGE_PAUSE)
        {
			man.fSetupTemper -= 0.1;
			if(man.fSetupTemper < SETUP_TEMPER_MIN) man.fSetupTemper = SETUP_TEMPER_MIN;
			sprintf(man.stringTemp, "%f", man.fSetupTemper);
            SetTextValueLen(VAR_ADDR_SETUPTEMPER, man.stringTemp, 4);

            man.fLowTemper = man.fSetupTemper - 1.0;
            man.fHighTemper = man.fSetupTemper + 1.0;
            sprintf(man.stringTemp, "%f", man.fLowTemper);
            SetTextValueLen(VAR_ADDR_SETUPTEMPER_LOW, man.stringTemp, 4);
            sprintf(man.stringTemp, "%f", man.fHighTemper);
            SetTextValueLen(VAR_ADDR_SETUPTEMPER_HIGH, man.stringTemp, 4);
        }
        else if(man.curPage == PAGE_CALIBRATION)
        {
            SetTextFontColor(VAR_ADDR_CALIB_INNEROXYGEN_OFFSET_PTR+man.calibIndex*0x0010, COLOR_WHITE);
            man.calibIndex--;
            if(man.calibIndex < 0)
                man.calibIndex = 9;
            SetTextFontColor(VAR_ADDR_CALIB_INNEROXYGEN_OFFSET_PTR+man.calibIndex*0x0010, COLOR_RED);
        }
        else if(man.curPage == PAGE_ADJUSTED)
        {
            SetTextFontColor(VAR_ADDR_ADJUST_INNEROXYGEN_P_PTR+man.adjustPIDIndex*0x0010, COLOR_WHITE);
            man.adjustPIDIndex--;
            if(man.adjustPIDIndex < 0)
                man.adjustPIDIndex = 8;
            SetTextFontColor(VAR_ADDR_ADJUST_INNEROXYGEN_P_PTR+man.adjustPIDIndex*0x0010, COLOR_RED);
        }
        else if(man.curPage == man.pageModeSelect && !man.workMode)
        {
            man.oxygenMode = MODE_CPAP; //进入CPAP模式
            SetTextValueLen(VAR_ADDR_MODE, CAPA_MODE_STRING, 8);
            SetTextValueLen(VAR_ADDR_CURRENTMASKOXYGEN, TILDE_STRING, 2);
            SetTextValueLen(VAR_ADDR_CURRENTMASKTEMPER, TILDE_STRING, 4);
            man.curPage = PAGE_PAUSE;
            SetScreen(man.curPage);    
        }
	}

    if(key.rasingEdge&SW5_MASK)	//启动/停止
	{
		if(man.curPage == PAGE_PAUSE) //启动
        {
			man.curPage = PAGE_RUNNING;
            SetScreen(man.curPage);
            man.startFlag = 1;
            man.oxygenAdjustState = 0;

            man.powerOn = 1;
            man.powerOnCnt = POWERONNOALARMTIME;
            //man.powerOnCnt = 5;

//            SetTextValueLen(VAR_ADDR_WARNINGWORKING, OXYCURE_WORKING_STRING, 8);
//            SetTextFontColor(VAR_ADDR_WARNINGWORKING_PTR, COLOR_BLACK);

            if(man.alarmStatus&ALARM_OXYCURE_OXYGEN)
                SetTextFontColor(VAR_ADDR_CURRENTOXYGEN_PTR, COLOR_RED);    
            if(man.alarmStatus&ALARM_OXYCURE_TEMPER)
                SetTextFontColor(VAR_ADDR_CURRENTTEMPER_PTR, COLOR_RED);   

            //PID_UpdateSetPoint(&(man.plateHeatTemperPID), man.fSetupTemper*100);
            PID_UpdateSetPoint(&(man.plateHeatTemperPID), (man.fSetupTemper-3.0)*100);
            PID_UpdateSetPoint(&(man.lineHeatTemperPID), man.fSetupTemper*100);
//            cDebug("plate PID set point is %d\r\n", (int)(man.plateHeatTemperPID.SetPoint));
//            cDebug("line PID set point is %d\r\n", (int)(man.lineHeatTemperPID.SetPoint));
//
//            cDebug("man.fSetupOxygen = %f\r\n", man.fSetupOxygen);
//            cDebug("man.fCurInnerOxygen = %f\r\n", man.fCurInnerOxygen);
        }
        else if(man.curPage == PAGE_CALIBRATION)//保存校准参数
        {
            IapErase(EEPROM_BASEADDR_DEFAULT);
            os_wait(K_TMO, 2, 0); //擦除需要等待4~6ms，这里等待10ms
            IapErase(EEPROM_BASEADDR_CALIB);
            os_wait(K_TMO, 2, 0); //擦除需要等待4~6ms，这里等待10ms
            IapProgram(EEPROM_BASEADDR_DEFAULT, 0xA5);
            EEPROM_WriteBytes(EEPROM_BASEADDR_CALIB, (uint8_t*)(man.pCalib), sizeof(float)*10);
            man.curPage = PAGE_PAUSE;
            SetScreen(man.curPage);
        }
        else if(man.curPage == PAGE_ADJUSTED)//保存PID参数
        {
            IapErase(EEPROM_BASEADDR_ADJUST);
            os_wait(K_TMO, 2, 0); //擦除需要等待4~6ms，这里等待10ms
            EEPROM_WriteBytes(EEPROM_BASEADDR_ADJUST, (uint8_t*)(man.pAdjustPID), sizeof(float)*3);
            EEPROM_WriteBytes(EEPROM_BASEADDR_ADJUST+sizeof(float)*3, (uint8_t*)(man.pAdjustPID+1), sizeof(float)*3);
            EEPROM_WriteBytes(EEPROM_BASEADDR_ADJUST+sizeof(float)*6, (uint8_t*)(man.pAdjustPID+2), sizeof(float)*3);
            man.curPage = PAGE_PAUSE;
            SetScreen(man.curPage);
        }
        //else //暂停
		else if(man.curPage == PAGE_RUNNING) //暂停
        {
			man.curPage = PAGE_PAUSE;
            SetScreen(man.curPage);

            man.startFlag = 0;        
            stepMotor_Stop();//停止步进电机
            HeatPlate_Adjust(0);//停止加热盘加热
            HeatLine_Adjust(0); //停止加热线加热

            SetTextFontColor(VAR_ADDR_CURRENTOXYGEN_PTR, COLOR_GREEN);
            SetTextFontColor(VAR_ADDR_CURRENTTEMPER_PTR, COLOR_GREEN);   
        }
        else if(man.curPage == PAGE_UNPLUGOXYGEN)
        {
            if(man.autoCalibState == 1)
            {
                man.autoCalibState = 2;

                man.curPage = PAGE_CALIBRATING; 
                SetScreen(man.curPage);
            }
            else if(man.autoCalibState == 5)
            {
                man.autoCalibState = 6;

                man.curPage = PAGE_CALIBRATING; 
                SetScreen(man.curPage);
            }

            man.beeperTimes = 1;
        }
        else if(man.curPage == PAGE_UNPLUGAIR)
        {
            if(man.autoCalibState == 3)
            {
                man.autoCalibState = 4;

                man.curPage = PAGE_CALIBRATING; 
                SetScreen(man.curPage);
            }

            man.beeperTimes = 1;
        }
        else        
            cDebug("Do not press the Start/Stop key\r\n");

	}
    if(key.rasingEdge&SW7_MASK)	//静音
	{
        if(man.curPage == PAGE_CALIBRATION || man.curPage == PAGE_ADJUSTED)
        {
            man.curPage = man.prevPage;
			SetScreen(man.curPage);    
        }
        else if(man.startFlag && man.alarmStatus&ALARM_OXYCURE)
        {
            man.silenceStatus |= 0x80;
            man.silenceTime = SILENCETIME;
            SetTextValueLen(VAR_ADDR_WARNINGWORKING, SILENCE_STATUS_STRING, 8);
            SetTextFontColor(VAR_ADDR_WARNINGWORKING_PTR, COLOR_BLACK);    
        }
        else if(man.curPage == PAGE_CALIBRATING)
        {
            man.autoCalibFlag = 0;
            man.autoCalibState = 0;
            StepMotor_Stop();

            man.curPage = PAGE_PAUSE; 
            SetScreen(man.curPage);
        }
        
	}

    man.comboKeyAutoCalibCnt = 0;
    man.comboKeyCalibCnt = 0;
    man.comboKeyAdjustCnt = 0;
    man.comboKeyDefaultResetCnt = 0;
    man.comboKeyModeChangeCnt = 0;
    man.comboKeyBeeperStopCnt = 0;
    man.comboKeyWorkModeCnt = 0;   
}

void Key_Process_Combo(void)
{
    if(key.rasingEdge&SW1_MASK) //组合键+氧浓度增加      浏览页面
	{
//        static uint8_t page = 0;
//        if(page++ > PAGE_CALIBRATING) page = 0;                   
//		SetScreen(page);

        man.comboKeyBeeperStopCnt++;
		if(man.comboKeyBeeperStopCnt >= COMBOKEYBEEPERSTOPCOUNT) //蜂鸣器禁止
        {
            man.comboKeyBeeperStopCnt = 0;

            man.beeperStopFlag = !man.beeperStopFlag;

            cDebug("Forbid the beeper……\r\n");
        }        
	}
    else
        man.comboKeyBeeperStopCnt = 0;
	
    if(key.rasingEdge&SW2_MASK) //组合键+氧浓度减少     自动校准
	{
        if(!man.startFlag)
        {
            man.comboKeyAutoCalibCnt++;
    		if(man.comboKeyAutoCalibCnt >= COMBOKEYAUTOCALIBCOUNT)
            {
                if(man.curPage == PAGE_CALIBRATION) //校准页面
                    man.autoCalibFlag |= 0x02;  //进入自动校准程序
                else
                    man.autoCalibFlag |= 0x01;  //进入自动校准程序，无99%校准
                
                man.beeperTimes = 1;

                man.prevPage = man.curPage;
    			man.comboKeyAutoCalibCnt = 0;
                man.curPage = PAGE_CALIBRATING;//    提示正在校准
    		    SetScreen(man.curPage);
    
                cDebug("The Calibration program is running……\r\n");
            }
        }
	}
    else
        man.comboKeyAutoCalibCnt = 0;

	if(key.rasingEdge&SW3_MASK)	//组合键+温度增加    调节参数设置
	{
        man.comboKeyAdjustCnt++;
		if(man.comboKeyAdjustCnt >= COMBOKEYADJUSTCOUNT)
        {
            man.prevPage = man.curPage;
			man.comboKeyAdjustCnt = 0;
            man.curPage = PAGE_ADJUSTED;//    PAGE_ADJUSTED
		    SetScreen(man.curPage);
        }
	}
    else
        man.comboKeyAdjustCnt = 0;

	if(key.rasingEdge&SW4_MASK) //组合键+温度减少    校准参数设置
	{
        if(man.curPage != PAGE_CALIBRATION)// && man.curPage == PAGE_PAUSE)
        {  
			man.comboKeyCalibCnt++;
			if(man.comboKeyCalibCnt >= COMBOKEYCALIBCOUNT)
            {
                man.prevPage = man.curPage;
				man.comboKeyCalibCnt = 0;
                man.curPage = PAGE_CALIBRATION;
			    SetScreen(man.curPage);
                //cDebug("this is PAGE_CALIBRATION!!!\r\n");
            }
        }
	}
    else
        man.comboKeyCalibCnt = 0;

	if(key.rasingEdge&SW5_MASK)	//组合键+启动/停止    恢复出厂设置
	{
        if(!man.startFlag)
        {
            man.comboKeyDefaultResetCnt++;
    		if(man.comboKeyDefaultResetCnt >= COMBOKEYDEFAULTRESETCOUNT)
            {
                man.prevPage = man.curPage;
    			man.comboKeyDefaultResetCnt = 0;
    
                IapErase(EEPROM_BASEADDR_DEFAULT);
                os_wait(K_TMO, 2, 0); //擦除需要等待4~6ms，这里等待10ms
    
                man.curPage = PAGE_LOGO;
    		    SetScreen(man.curPage);

                cDebug("Reset Default Data...\r\n");
            }
        }
	}
    else
        man.comboKeyDefaultResetCnt = 0;

    if(key.rasingEdge&SW7_MASK)	//组合键+静音      模式切换
	{
        if(!man.startFlag)
        {
            man.comboKeyModeChangeCnt++;
    		if(man.comboKeyModeChangeCnt >= COMBOKEYMODECHANGECOUNT)
            {
                man.comboKeyModeChangeCnt = 0;

                cDebug("Mode Change\r\n");

                if(man.oxygenMode == MODE_ANABIOSIS)
                {
                    man.oxygenMode = MODE_OXYGENAIR; //进入空氧模式
                    SetTextValueLen(VAR_ADDR_MODE, OXYGEN_MODE_STRING, 8);
                }
                else if(man.oxygenMode == MODE_OXYGENAIR)
                {
                    if(man.workMode == 0) //三合一
                    {
                        man.oxygenMode = MODE_CPAP; //进入CPAP模式
                        SetTextValueLen(VAR_ADDR_MODE, CAPA_MODE_STRING, 8);
                    }
                    else
                    {
                        man.oxygenMode = MODE_ANABIOSIS; //进入复苏模式
                        SetTextValueLen(VAR_ADDR_MODE, RHINOBYON_MODE_STRING, 8);
                    }
                    SetTextValueLen(VAR_ADDR_CURRENTMASKOXYGEN, TILDE_STRING, 2);
                    SetTextValueLen(VAR_ADDR_CURRENTMASKTEMPER, TILDE_STRING, 4);   
                }
                else if(man.oxygenMode == MODE_CPAP)
                {
                    man.oxygenMode = MODE_ANABIOSIS; //进入复苏模式
                    SetTextValueLen(VAR_ADDR_MODE, RHINOBYON_MODE_STRING, 8);    
                }
            }
        }
	}
}

void Key_Process(void)
{
	if(key.rasingEdge)//上升沿
	{
        //cDebug("the key is 0x%x\r\n", (int)key.rasingEdge);

		if(!(key.curKey&SW6_MASK))	//组合键没按下
		{
		    Key_Process_NoCombo();	
		}
		else //组合键按下了
		{
            static flag = 0;

		    Key_Process_Combo();

//            if((key.curKey&(SW3_MASK|SW4_MASK)) == (SW3_MASK|SW4_MASK))	//组合键+静音+氧浓度增加+氧浓度减少      三合一模式切换
//            {
//                if(key.rasingEdge&SW7_MASK)
//                {
//                    man.comboKeyWorkModeCnt++; 
//                }
//            }

            if((key.curKey&(SW1_MASK|SW2_MASK)) == (SW1_MASK|SW2_MASK))	//组合键+静音+氧浓度增加+氧浓度减少      三合一模式切换
            {
                if(key.rasingEdge&SW7_MASK)
                {
                    if(MIN2 == 0)
                        man.comboKeyWorkModeCnt++;
                    if(man.comboKeyWorkModeCnt > COMBOKEYWORKMODECOUNT)
                    {
                        man.comboKeyWorkModeCnt = 0;

                        if(man.workMode == 0) man.workMode = 1; //二合一
                        else if(man.workMode == 1) man.workMode = 0;  //三合一
    
                        cDebug("WorkMode change! man.workMode = %d\r\n", (int)man.workMode);
    
                        IapErase(EEPROM_BASEADDR_WORKMODE);
                        IapProgram(EEPROM_BASEADDR_WORKMODE, man.workMode);

                        man.curPage = PAGE_LOGO;
    		            SetScreen(man.curPage);
                    } 
                }
            }
        }	

		if(key.rasingEdge&SW6_MASK)	//组合键
        {
            ;
        }	
	}
}

void Key_Process_LongPress(void)
{
	//松开按键，清长按计数器
	if(key.fallingEdge)
	{
		if(key.fallingEdge&SW1_MASK)
			key.oxygenChangeCnt = 0;
		if(key.fallingEdge&SW2_MASK)
			key.oxygenChangeCnt = 0;
		if(key.fallingEdge&SW3_MASK)
			key.temperChangeCnt = 0;	
		if(key.fallingEdge&SW4_MASK)
			key.temperChangeCnt = 0;				
	}

   	//长按
	if(key.curKey)
	{
		if(key.curKey&SW1_MASK) //氧浓度增加
		{
            if(man.curPage == PAGE_PAUSE)
            {
    			key.oxygenChangeCnt++;
    			if(key.oxygenChangeCnt >= KEY_CONTINUAL_COUNT)
    			{
    				man.iSetupOxygen += 2;
    				if(man.iSetupOxygen > SETUP_OXYGEN_MAX) man.iSetupOxygen = SETUP_OXYGEN_MAX;
                    man.fSetupOxygen = man.iSetupOxygen;
    				Int2String(man.iSetupOxygen, man.sSetupOxygen);
    				SetTextValueLen(VAR_ADDR_SETUPOXYGEN, man.sSetupOxygen, 2);
    
                    man.iLowOxygen = man.iSetupOxygen - 5;
                    man.iHighOxygen = man.iSetupOxygen + 5;
                    Int2String(man.iLowOxygen, man.sSetupOxygen);
    				SetTextValueLen(VAR_ADDR_SETUPOXYGEN_LOW, man.sSetupOxygen, 2);
                    Int2String(man.iHighOxygen, man.sSetupOxygen);
    				SetTextValueLen(VAR_ADDR_SETUPOXYGEN_HIGH, man.sSetupOxygen, 2);
    			}
            }
		}
		if(key.curKey&SW2_MASK)  //氧浓度减少
		{
            if(man.curPage == PAGE_PAUSE)
            {
    			key.oxygenChangeCnt++;
    			if(key.oxygenChangeCnt >= KEY_CONTINUAL_COUNT)
    			{
    				man.iSetupOxygen -= 2;
    				if(man.iSetupOxygen < SETUP_OXYGEN_MIN) man.iSetupOxygen = SETUP_OXYGEN_MIN;
                    man.fSetupOxygen = man.iSetupOxygen;
    				Int2String(man.iSetupOxygen, man.sSetupOxygen);
    				SetTextValueLen(VAR_ADDR_SETUPOXYGEN, man.sSetupOxygen, 2);
    
                    man.iLowOxygen = man.iSetupOxygen - 5;
                    man.iHighOxygen = man.iSetupOxygen + 5;
                    Int2String(man.iLowOxygen, man.sSetupOxygen);
    				SetTextValueLen(VAR_ADDR_SETUPOXYGEN_LOW, man.sSetupOxygen, 2);
                    Int2String(man.iHighOxygen, man.sSetupOxygen);
    				SetTextValueLen(VAR_ADDR_SETUPOXYGEN_HIGH, man.sSetupOxygen, 2);
    			}
            }
		}
		if(key.curKey&SW3_MASK)  //温度增加
		{
            if(man.curPage == PAGE_PAUSE)
            {
    			key.temperChangeCnt++;
    			if(key.temperChangeCnt >= KEY_CONTINUAL_COUNT)
    			{
    				man.fSetupTemper += 0.2;
    				if(man.fSetupTemper > SETUP_TEMPER_MAX) man.fSetupTemper = SETUP_TEMPER_MAX;
    				sprintf(man.stringTemp, "%f", man.fSetupTemper);
                    SetTextValueLen(VAR_ADDR_SETUPTEMPER, man.stringTemp, 4);

                    man.fLowTemper = man.fSetupTemper - 1.0;
                    man.fHighTemper = man.fSetupTemper + 1.0;
                    sprintf(man.stringTemp, "%f", man.fLowTemper);
                    SetTextValueLen(VAR_ADDR_SETUPTEMPER_LOW, man.stringTemp, 4);
                    sprintf(man.stringTemp, "%f", man.fHighTemper);
                    SetTextValueLen(VAR_ADDR_SETUPTEMPER_HIGH, man.stringTemp, 4);
    			}
            }
		}	
		if(key.curKey&SW4_MASK)  //温度减少
		{
            if(man.curPage == PAGE_PAUSE)
            {
    			key.temperChangeCnt++;
    			if(key.temperChangeCnt >= KEY_CONTINUAL_COUNT)
    			{
    				man.fSetupTemper -= 0.2;
    				if(man.fSetupTemper < SETUP_TEMPER_MIN) man.fSetupTemper = SETUP_TEMPER_MIN;
    				sprintf(man.stringTemp, "%f", man.fSetupTemper);
                    SetTextValueLen(VAR_ADDR_SETUPTEMPER, man.stringTemp, 4);

                    man.fLowTemper = man.fSetupTemper - 1.0;
                    man.fHighTemper = man.fSetupTemper + 1.0;
                    sprintf(man.stringTemp, "%f", man.fLowTemper);
                    SetTextValueLen(VAR_ADDR_SETUPTEMPER_LOW, man.stringTemp, 4);
                    sprintf(man.stringTemp, "%f", man.fHighTemper);
                    SetTextValueLen(VAR_ADDR_SETUPTEMPER_HIGH, man.stringTemp, 4);
    			}
            }
		}
    }
}
