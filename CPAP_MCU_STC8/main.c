#include "uart.h"
#include "timer.h"
#include "io.h"
#include "eeprom.h"
#include "stepmotor.h"
#include "tm770x.h"

#include "HMI/hmi_driver.h"
#include "HMI/cmd_queue.h"
#include "HMI/cmd_process.h"

#include "management.h"
#include "misc.h"

#include <string.h>
#include <stdlib.h>

//RTX51 Tiny的时钟滴答为Fosc/12/INT_CLOCK
//Fosc=32MHz,INT_CLOCK=13333,所以RTX51 Tiny的时钟滴答约为5ms
//Fosc=24MHz,INT_CLOCK=10000,所以RTX51 Tiny的时钟滴答约为5ms
//如果用os_wait()延时，大约200个时钟滴答为1s，即os_wait(K_TMO, 200, 0)

//-----------------------------------------------

/* main program */
void startupTask(void) _task_ TASK_STARTUP 
{
	//---------------初始化------------------------------------
	IO_Init();
	Timer_Init(); //定时器初始化

	queue_reset(&lcd_que);
	queue_reset(&user_que);

	Uart_Init();//串口初始化

	//AT24CXX_Init();

    EA = 1;//开启全局中断
    //while(1)
    cDebug("hello CPAP!\r\n");
	//Uart_SendString("hello CPAP!\r\n");

//    while(1)
//    {
//        os_wait(K_TMO, 200, 0);
//        os_wait(K_TMO, 200, 0);
//        cDebug("hello CPAP!\r\n");
//        LEDM = !LEDM;    
//    }

    TM770X_Init(CHIP_ALL);
    InitMan();

    //WDT_CONTR = 0x3C;//开看门狗，溢出时间1.1377s

    //---------------创建任务-----------------------------
	os_create_task(TASK_UI); 	//创建任务1

	//---------------删除该任务-----------------------------
 	os_delete_task(TASK_STARTUP);	//删除自己(task0),使task0退出任务链表
}

void UITask(void) _task_ TASK_UI
{
    uint8_t size = 0;
    uint8_t i;
    uint32_t cnt;

//    cDebug("UITask is running...\r\n");

    os_wait(K_TMO, 200, 0);
    os_wait(K_TMO, 200, 0);
//    os_wait(K_TMO, 200, 0);
//    os_wait(K_TMO, 200, 0);
//    os_wait(K_TMO, 200, 0);

    //页面参数初始化
    //氧浓度
	Int2String(man.iSetupOxygen, man.sSetupOxygen);
	SetTextValueLen(VAR_ADDR_SETUPOXYGEN, man.sSetupOxygen, 2);
    man.iLowOxygen = man.iSetupOxygen - 5;
    man.iHighOxygen = man.iSetupOxygen + 5;
    Int2String(man.iLowOxygen, man.sSetupOxygen);
	SetTextValueLen(VAR_ADDR_SETUPOXYGEN_LOW, man.sSetupOxygen, 2);
    Int2String(man.iHighOxygen, man.sSetupOxygen);
	SetTextValueLen(VAR_ADDR_SETUPOXYGEN_HIGH, man.sSetupOxygen, 2);
    //温度
    sprintf(man.stringTemp, "%f", man.fSetupTemper);
    SetTextValueLen(VAR_ADDR_SETUPTEMPER, man.stringTemp, 4);
    man.fLowTemper = man.fSetupTemper - 1.0;
    man.fHighTemper = man.fSetupTemper + 1.0;
    sprintf(man.stringTemp, "%f", man.fLowTemper);
    SetTextValueLen(VAR_ADDR_SETUPTEMPER_LOW, man.stringTemp, 4);
    sprintf(man.stringTemp, "%f", man.fHighTemper);
    SetTextValueLen(VAR_ADDR_SETUPTEMPER_HIGH, man.stringTemp, 4);

    //校准页面参数初始化
    for(i=0;i<10;i++)
    {
        char s[10];
        sprintf(s, "%f", *(man.pCalib+i));
        SetTextValueLen(VAR_ADDR_CALIB_INNEROXYGEN_OFFSET+i*0x0004, s, 6);
        os_wait(K_TMO, 1, 0);
    }
    SetTextFontColor(VAR_ADDR_CALIB_INNEROXYGEN_OFFSET_PTR+man.calibIndex*0x0010, COLOR_RED);

    //PID调节页面参数初始化
    for(i=0;i<9;i++)
    {
        char s[10];
        sprintf(s, "%f", *(&((man.pAdjustPID+i/3)->Proportion)+i%3));
        SetTextValueLen(VAR_ADDR_ADJUST_INNEROXYGEN_P+i*0x0004, s, 6);
        os_wait(K_TMO, 1, 0);
    }
    SetTextFontColor(VAR_ADDR_ADJUST_INNEROXYGEN_P_PTR+man.adjustPIDIndex*0x0010, COLOR_RED);

    //读取RTC时间
//    GetRTC();
//    cnt = 0;
//    while(!(man.autoCalibFlag&0x80) && (cnt++ < 50000))
//    {
//        uint8_t size = queue_find_cmd(&lcd_que, lcd_cmd_buffer, CMD_MAX_SIZE); //从缓冲区中获取一条指令			       
//		if(size>0)//接收到指令
//		{
//			ProcessLCDMessage(lcd_cmd_buffer, size);//指令处理
//		}
//    }
//    if(cnt >= 50000)
//    {
//        cDebug("Can not read the rtc!\r\n");
//    }

    os_create_task(TASK_ADC);	//创建任务2
    os_create_task(TASK_ALARM);	//创建任务2
    os_create_task(TASK_TIMER);	//创建任务2 
       
    SetScreen(man.curPage);

    while (1)                       //loop
	{
        os_wait(K_TMO, 10, 0);

        //键盘
        Key_Scan();
        //if(!(man.alarmStatus&ALARM_TEMPER))//温度超过43℃需要停机
        { 
    		Key_Process();
            Key_Process_LongPress(); //长按按键处理
        }

		//解析串口数据
		size = queue_find_cmd(&user_que, user_cmd_buffer, CMD_MAX_SIZE); //从缓冲区中获取一条指令			       
		if(size>0)//接收到指令
		{
			ProcessUserMessage(user_cmd_buffer, size);//指令处理
		}
        size = queue_find_cmd(&lcd_que, lcd_cmd_buffer, CMD_MAX_SIZE); //从缓冲区中获取一条指令			       
		if(size>0)//接收到指令
		{
			ProcessLCDMessage(lcd_cmd_buffer, size);//指令处理
		}

        AlarmCheck();

		LEDM = !LEDM;
		//MBEEP = !MBEEP; 
	}
}

void ADCTask(void) _task_ TASK_ADC
{
//    cDebug("ADCTask is running...\r\n");
//
//    cDebug("man.autoCalibFlag = %d\r\n", (int)man.autoCalibFlag);
    
    while(1)
    {
        os_wait(K_TMO, 10, 0);

        //氧浓度和温度数据采集
        ADCGet();

        //氧浓度调节
        if(man.startFlag)
        {
            OxygenPercentControl();

            TemperControl();
        }
        else 
        {
            //暂停时要关闭发热装置
            HeatPlate_Adjust(0);
            HeatLine_Adjust(0);

            if(man.oxygenMode != MODE_NONE)//自动校准
            {
                if(man.autoCalibFlag&0x01)
                    AutoCalibration(0);//不校准99%
                    //AutoCalibration(1);
                else if(man.autoCalibFlag&0x02)
                    AutoCalibration(1);
            }       
        }
        //MBEEP = !MBEEP;
        //LEDM = !LEDM;
    }
   
}

void AlarmTask(void) _task_ TASK_ALARM
{
//    cDebug("AlarmTask is running...\r\n");

    while(1)
    {
        //喂狗
		//WDT_CONTR = 0x3C;

        if(man.alarmStatus&ALARM_NO_MASK 
            || (!man.powerOn && man.alarmStatus&ALARM_OXYCURE && man.startFlag && !(man.silenceStatus&0x80)))
        {
            os_wait(K_TMO, 100, 0);
            man.beeperAlarm = 1;
            os_wait(K_TMO, 100, 0);
            man.beeperAlarm = 0;

            //man.beeperAlarm = 1;
            //LEDM = !LEDM;   
        }
        else if(man.beeperTimes)
        {
            if(man.beeperTimes)
                man.beeperTimes--;

            os_wait(K_TMO, 200, 0);
            man.beeperAlarm = 1;
            os_wait(K_TMO, 200, 0);
            man.beeperAlarm = 0;
        }
        else
            //man.beeperAlarm = 0;
            os_wait(K_TMO, 10, 0);

        //LEDM = !LEDM;
        //MBEEP = !MBEEP;    
    }    
}

void TimerTask(void) _task_ TASK_TIMER
{
    //int16_t powerOnCnt = POWERONNOALARMTIME;

//    cDebug("TimerTask is running...\r\n");

    while(1)
    {
        os_wait(K_TMO, 200, 0);
    
        if(man.powerOn)
        {
            man.powerOnCnt--;
            if(man.powerOnCnt < 0)
                man.powerOn = 0;
        }

        if(man.silenceStatus&0x80)
        {
            man.silenceTime--;
            if(man.silenceTime < 0)//静音时间到
            {
                man.silenceStatus = 0;

                //恢复氧疗状态
                if(man.alarmStatus&ALARM_OXYCURE)
                {
                    SetTextValueLen(VAR_ADDR_WARNINGWORKING, OXYCURE_WARNING_STRING, 8);
                    SetTextFontColor(VAR_ADDR_WARNINGWORKING_PTR, COLOR_RED); 
                }
                else
                {
                    SetTextValueLen(VAR_ADDR_WARNINGWORKING, OXYCURE_WORKING_STRING, 8);
                    SetTextFontColor(VAR_ADDR_WARNINGWORKING_PTR, COLOR_BLACK);    
                }
            }   
        }

        if(man.userTimeOutFlag)
        {
            man.userTimeCnt--;
            if(man.userTimeCnt <= 0)
                man.userTimeOutFlag = 0;    
        }
    }
}
