#ifndef __TM770X_H
#define __TM770X_H

#include "common.h"

#define CHIP0_TM7706 0x01
#define CHIP1_TM7707 0x02
#define CHIP2_TM7707 0x04

#define CHIP_ALL     CHIP0_TM7706|CHIP1_TM7707|CHIP2_TM7707

//通信寄存器功能
#define TM7706_REG_COM		0x00
#define TM7706_REG_SETUP	0x10
#define TM7706_REG_CLK		0x20
#define TM7706_REG_DATA	    0x30
#define TM7706_REG_TEST	    0x40
#define TM7706_REG_NONE	    0x50
#define TM7706_REG_OFFSET	0x60
#define TM7706_REG_GAIN	    0x70

#define TM7707_REG_COM		0x00
#define TM7707_REG_SETUP	0x10
#define TM7707_REG_FILTER_H	0x20
#define TM7707_REG_DATA	    0x30
#define TM7707_REG_TEST	    0x40
#define TM7707_REG_FILTER_L 0x50
#define TM7707_REG_CALIB_Z	0x60
#define TM7707_REG_CALIB_F  0x70

#define REG_MASK    0x70

#define READ_REG    0x08
#define CHANNAL_1   0x00
#define CHANNAL_2   0x01

//设置寄存器功能
#define MODE_NORMAL     0x00
#define MODE_CALIB_SELF 0x40
#define MODE_CALIB_ZERO 0x80
#define MODE_CALIB_FULL 0xC0

#define GAIN_1          0x00
#define GAIN_2          0x08
#define GAIN_4          0x10
#define GAIN_8          0x18
#define GAIN_16         0x20
#define GAIN_32         0x28
#define GAIN_64         0x30
#define GAIN_128        0x38

#define POLARITY_B      0x00 //双极性
#define POLARITY_U      0x40

#define BUFFER_USED     0x00
#define BUFFER_NONE     0x02
					  
uint8_t TM770X_ReadRegister(uint8_t chip, uint8_t *ReadData, uint8_t RegLen);		//指定地址读取一个字节
void TM770X_WriteRegister(uint8_t chip, uint8_t ByteData);		//指定地址写入一个字节

void TM770X_SyncSPI(uint8_t chip);
uint8_t TM770X_ReadData(uint8_t ch, uint8_t *pReadData);
uint8_t TM770X_ReadCalibData(uint8_t ch, uint8_t calibType, uint8_t *pReadData);
void TM770X_Init(uint8_t chip); //初始化IIC
  
#endif
















