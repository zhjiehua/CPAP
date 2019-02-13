#ifndef _CMD_PROCESS_H
#define _CMD_PROCESS_H
#include "hmi_driver.h"

#define CMD_PARAMETER_SETTING			0X81  //设置参数
#define CMD_MOTORINIT_START				0X82  //启动马达初始化
#define CMD_MOTORINIT_STOP				0X83  //停止马达初始化
#define CMD_WAVECONTROL_SETTING			0X84  //设定波长控制字
#define CMD_MOTORSTEP_TURNING			0X85  //马达转动指定步数
#define CMD_MOTOR_TURNING				0X86  //马达连续自动转动
#define CMD_PARAMETER_READ				0X88  //读取参数
#define CMD_VERSION_READ                0X89  //读取分控软件版本
#define CMD_AU_READ		                0X8A  //读取AU值


#define PTR2U16(PTR) ((((uint8 *)(PTR))[0]<<8)|((uint8 *)(PTR))[1])  //从缓冲区取16位数据
#define PTR2U32(PTR) ((((uint8 *)(PTR))[0]<<24)|(((uint8 *)(PTR))[1]<<16)|(((uint8 *)(PTR))[2]<<8)|((uint8 *)(PTR))[3])  //从缓冲区取32位数据

void ProcessLCDMessage(uint8 *msg, uint16 size);
void ProcessUserMessage(uint8 *msg, uint16 size);

#endif
