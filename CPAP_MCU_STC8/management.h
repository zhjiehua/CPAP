#ifndef __MANAGEMENT_H__
#define __MANAGEMENT_H__

#include  "common.h"
#include "PID.h"

//使用三合一是否可以用键盘修改
#define WORKMODE_KEYCHANGE      1

#define OS_TICK    (200)
#define TM7706_OUTPUT_HZ 15 //tm7706的输出更新率，用于延时os_wait()


//温度ADC采集用
#define TEMPER_ADC_INPUT_VOLTAGE_RANGE_MV  20.0 //ADC输入电压20mV范围，由于128增益，2.5V参考电压，2.5V/128=20mV
#define TEMPER_ADC_PULLUP_RESISTOR_OHM     4990.0 //上拉电阻
#define TEMPER_ADC_PULLDOWN_RESISTOR_OHM   100.0 //下拉电阻
#define TEMPER_ADC_POWER_VOLTAGE_MV        5000.0 //ADC电源电压，通过上拉电阻和下拉电阻（或者Pt100）分压作为TM7707的输入
//温度ADC负端的输入电压
#define TEMPER_ADC_NEGATIVE_INPUT_VOLTAGE_MV  (TEMPER_ADC_POWER_VOLTAGE_MV*TEMPER_ADC_PULLDOWN_RESISTOR_OHM/(TEMPER_ADC_PULLUP_RESISTOR_OHM+TEMPER_ADC_PULLDOWN_RESISTOR_OHM)) 

//氧浓度ADC采集用
#define OXYGEN_ADC_INPUT_VOLTAGE_RANGE_MV  80.0 //ADC输入电压80mV范围，由于32增益，2.5V参考电压，2.5V/32=80mV

//ADC采集缓冲区大小
#define OXYGENBUFFER_SIZE 5
#define TEMPERBUFFER_SIZE OXYGENBUFFER_SIZE

//串口屏页面编码
#define PAGE_LOGO           0
#define PAGE_PAUSE          1
#define PAGE_RUNNING        2
#define PAGE_MODESELECT     3
#define PAGE_CALIBRATION    4
#define PAGE_ADJUSTED       5
#define PAGE_OVERTEMPER43   6
#define PAGE_UNPLUGOXYGEN   7
#define PAGE_UNPLUGAIR      8
#define PAGE_REPLACE_S1     9
#define PAGE_REPLACE_S2     10
#define PAGE_CALIBRATING    11
#define PAGE_CALIBCOMPLETE  12
#define PAGE_MODESELECT_3   13

//串口屏颜色编码
//#define COLOR_RED 0xD800
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x0600
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF

//串口屏变量地址
#define VAR_ADDR_SETUPOXYGEN 				0x0000
#define VAR_ADDR_SETUPTEMPER         		0x0002

#define VAR_ADDR_CURRENTMASKOXYGEN			0x0006
#define VAR_ADDR_CURRENTMASKTEMPER      	0x0008
#define VAR_ADDR_CURRENTOXYGEN 		        0x000C
#define VAR_ADDR_CURRENTTEMPER              0x000E

#define VAR_ADDR_SETUPOXYGEN_LOW 			0x0012
#define VAR_ADDR_SETUPOXYGEN_HIGH 			0x0014
#define VAR_ADDR_SETUPTEMPER_LOW 	        0x0016
#define VAR_ADDR_SETUPTEMPER_HIGH	        0x001A

#define VAR_ADDR_MODE		 				0x0050
#define VAR_ADDR_WARNINGFLUX 				0x0060
#define VAR_ADDR_WARNINGCONNECT				0x0070
#define VAR_ADDR_WARNINGWORKING				0x0080

#define VAR_ADDR_CALIB_INNEROXYGEN_OFFSET	0x0400
#define VAR_ADDR_ADJUST_INNEROXYGEN_P	    0x0600

//描述指针地址
#define VAR_ADDR_WARNINGFLUX_PTR 				0x00F0
#define VAR_ADDR_WARNINGCONNECT_PTR				0x0100
#define VAR_ADDR_WARNINGWORKING_PTR				0x0110

#define VAR_ADDR_CURRENTMASKOXYGEN_PTR			0x00C0
#define VAR_ADDR_CURRENTMASKTEMPER_PTR	        0x00D0
#define VAR_ADDR_CURRENTOXYGEN_PTR 		        0x0090
#define VAR_ADDR_CURRENTTEMPER_PTR              0x00A0

#define VAR_ADDR_CALIB_INNEROXYGEN_OFFSET_PTR   0x0800
#define VAR_ADDR_ADJUST_INNEROXYGEN_P_PTR       0x0A00

//设置限制
#define SETUP_OXYGEN_MIN  21 
#define SETUP_OXYGEN_MAX  99
#define SETUP_TEMPER_MIN  30.0 
#define SETUP_TEMPER_MAX  40.0

#define TEMPER_MIN        15.0
#define TEMPER_MAX        43.0

#define UNCONNECT_TEMPER_MAX    (79.0-10.0)
//#define TEMPER_DIFFERENCE       (4.0)
#define TEMPER_DIFFERENCE       (24.0)

//按键长按计数器
#define KEY_CONTINUAL_COUNT 20

//工作模式
#define MODE_NONE       0
#define MODE_ANABIOSIS  1
#define MODE_OXYGENAIR  2
#define MODE_CPAP       3

//报警类型
#define ALARM_TEMPER            0x10
#define ALARM_FLUX              0x08
#define ALARM_CONNECT           0x04
#define ALARM_OXYCURE_OXYGEN    0x02
#define ALARM_OXYCURE_TEMPER    0x01

#define ALARM_NO_MASK           (ALARM_TEMPER|ALARM_FLUX|ALARM_CONNECT)
#define ALARM_OXYCURE           (ALARM_OXYCURE_OXYGEN|ALARM_OXYCURE_TEMPER)

//EEPROM ADDR
#define EEPROM_BASEADDR_DEFAULT     0x2000
#define EEPROM_BASEADDR_CALIB       0x2200
#define EEPROM_BASEADDR_ADJUST      0x2400
#define EEPROM_BASEADDR_AUTOCALIB   0x2600
#define EEPROM_BASEADDR_WORKMODE    0x2800

//时间
#define SILENCETIME             (3*60)   //静音3分钟  
#define POWERONNOALARMTIME      (10*60)  //15分钟定时器
//#define SILENCETIME             10 //(3*60)   //静音3分钟  
//#define POWERONNOALARMTIME      30 //(15*60)  //15分钟定时器

#define COMBOKEYBEEPERSTOPCOUNT 5
#define COMBOKEYAUTOCALIBCOUNT  2
#define COMBOKEYCALIBCOUNT      3
#define COMBOKEYADJUSTCOUNT     5
#define COMBOKEYDEFAULTRESETCOUNT     10
#define COMBOKEYMODECHANGECOUNT 3
#define COMBOKEYWORKMODECOUNT   15


#define AUTOCALIBRATIONPEROID   30 //自动校准周期，默认30天

typedef struct
{
    uint8_t oxygenMode; //工作模式 1：复苏模式；2：空氧模式；3：CPAP模式

    uint8_t startFlag;
    uint8_t powerOn; //开机置一，用于开机启动后15分钟才会氧疗报警
    int16_t powerOnCnt;//

    uint8_t curPage; //当前页面
    uint8_t prevPage; //上一页面
    uint8_t comboKeyAutoCalibCnt; //按10次组合键进入自动校准页面
    uint8_t comboKeyCalibCnt; //按10次组合键进入校准页面
    uint8_t comboKeyAdjustCnt; //按10次组合键进入PID调节页面
    uint8_t comboKeyDefaultResetCnt;
    uint8_t comboKeyModeChangeCnt;
    uint8_t comboKeyBeeperStopCnt;
    uint8_t comboKeyWorkModeCnt;

    uint8_t autoCalibFlag; //自动校准标志位
    uint8_t autoCalibState; //自动校准流程状态机
    uint32_t autoCalibDate; //下一次自动校准的日期

    /*运行状态            
    bit8: 1-运行中；0-停止
    bit7: 1-前进；0-后退
    bit6：1-使用步数
    */
	uint8_t stepMotorStatus; //电机状态
    uint16_t stepMotorStep; //步数

    //字符串显示
	char sSetupOxygen[2]; //设置氧浓度字符串
   	char sSetupTemperInteger[2]; //设置温度整数字符串
	char sSetupTemperDecimal[1]; //设置温度小数字符串
    char stringTemp[10];

	char sCurOxygen[2]; //当前氧浓度字符串
   	char sCurTemperInteger[2]; //当前温度整数字符串
	char sCurTemperDecimal[1]; //当前温度小数字符串

    //设置值
	uint8_t iSetupOxygen; //设置氧浓度
    float fSetupOxygen; //PID调节用
	float fSetupTemper; //设置温度

    uint8_t iLowOxygen; //最小氧浓度
    uint8_t iHighOxygen; //最大氧浓度
    float fLowTemper; //最小温度
    float fHighTemper; //最大温度

    //校准用
    float innerOxygenCalibOffset;
    float innerOxygenCalibRatio;
    float maskOxygenCalibOffset;
    float maskOxygenCalibRatio;
    float innerTemperCalibOffset;
    float innerTemperCalibRatio;
    float cubeTemperCalibOffset;
    float cubeTemperCalibRatio;
    float maskTemperCalibOffset;
    float maskTemperCalibRatio;
    float *pCalib;
    int8_t calibIndex;

    //当前实际数据
    uint8_t curInnerOxygen; //当前内部氧浓度
    uint8_t curMaskOxygen; //当前氧罩氧浓度
    float fCurInnerOxygen; //PID调节用
    float fCurMaskOxygen; //PID调节用
    float curInnerTemper; //当前内部温度
    float curCubeTemper; //当前气管温度
    float curMaskTemper; //当前氧罩温度

    //氧浓度采集数据
    uint16_t innerOxygenBufferADC[OXYGENBUFFER_SIZE];
    uint16_t maskOxygenBufferADC[OXYGENBUFFER_SIZE];
    uint16_t innerOxygenADC;
    uint16_t maskOxygenADC;

    //温度采集数据
    uint32_t innerTemperBufferADC[TEMPERBUFFER_SIZE];
    uint32_t cubeTemperBufferADC[TEMPERBUFFER_SIZE];
    uint32_t maskTemperBufferADC[TEMPERBUFFER_SIZE];
    uint32_t innerTemperADC;
    uint32_t cubeTemperADC;
    uint32_t maskTemperADC;

    //报警
    uint8_t beeperAlarm; //蜂鸣器响
    uint8_t alarmStatus; //bit4温度超过43℃报警; bit3-流量报警; bit2-连接报警; bit1-氧疗_氧浓度报警; bit0-氧疗_温度报警
    uint8_t alarmStatusLast;
    uint8_t alarmStatusRaisingEdge;
    uint8_t alarmStatusFallingEdge;

    uint8_t silenceStatus; //静音状态，bit7-进入静音状态,bit0-静音时间到
    int16_t silenceTime; //静音时间统计
    uint8_t beeperTimes;//蜂鸣器鸣叫几次

    //加热
    uint8_t heatControl; //加热控制 bit7-加热盘启动加热；bit3-加热线启动加热
    uint8_t plateHeatCnt; //加热盘计数器，主要用于计算PMM的百分比
    uint8_t plateHeatPWMPercent; //加热盘调节的百分比
    uint8_t lineHeatCnt; //加热线计数器，主要用于计算PMM的百分比
    uint8_t lineHeatPWMPercent; //加热线调节的百分比

    //PID
    PID_TypeDef oxygenPID;  //氧浓度PID
    PID_TypeDef plateHeatTemperPID; //加热盘温度PID
    PID_TypeDef lineHeatTemperPID; //加热线温度PID
    PID_TypeDef *pAdjustPID;
    int8_t adjustPIDIndex;

    uint8_t oxygenAdjustState; //氧浓度调节状态机

    uint8_t beeperStopFlag; //用于禁止蜂鸣器

    //模式选择
    uint8_t workMode; //0：三合一；1：二合一
    uint8_t pageModeSelect;
}Man_TypeDef;

//铂电阻
typedef struct
{
	float temperature;
	float resistorKOhm;
}Pt100_T_R_TypeDef;

//氧电池
typedef struct
{
    float oxygenPercent;
	float voltageMV;	
}OxygenSensor_V_P_TypeDef;

extern Man_TypeDef man;

void InitMan(void);

void OxygenPercentControl(void);
void TemperControl(void);

void ADCGet(void);
void AlarmCheck(void);

void AutoCalibration(uint8_t flag);

#endif
