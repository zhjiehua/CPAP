/*! 
 *  \file hmi_driver.h
 *  \brief 串口屏驱动文件
 *  \version 1.0
 *  \date 2012-2015
 *  \copyright 广州大彩光电科技有限公司
 */

#ifndef _HMI_DRIVER_
#define _HMI_DRIVER_

#define FIRMWARE_VER 917   //屏幕固件版本号

#define CRC16_ENABLE 0         /*!< 如果需要CRC16校验功能，修改此宏为1(此时需要在VisualTFT工程中配CRC校验)*/
#define CMD_MAX_SIZE 20 //64        /*!<单条指令大小，根据需要调整，尽量设置大一些*/
#define QUEUE_MAX_SIZE 60 //128  /*!< 指令接收缓冲区大小，根据需要调整，尽量设置大一些*/

#define LCD_CMD_HEAD	0xA55A

#define USER_CMD_HEAD 0XEE  //帧头
#define USER_CMD_TAIL 0XFFFCFFFF //帧尾

#include "common.h"

void SEND_DATA(uint8 c);
void BEGIN_CMD();
void END_CMD();
void SendNU8(uint8 *pData, uint16 nDataLen);
void SendNU16(uint16 *pData, uint16 nDataLen);

#if(CRC16_ENABLE)

void AddCRC16(uint8 *buffer, uint16 n, uint16 *pcrc);

/*!
*  \brief  检查数据是否符合CRC16校验
*  \param buffer 待校验的数据，末尾存储CRC16
*  \param n 数据长度，包含CRC16
*  \return 校验通过返回1，否则返回0
*/
uint16 CheckCRC16(uint8 *buffer, uint16 n);

#endif
void SendStrings(uint8 *str);

void ResetDevice(void);

void GetVersion();

void SetBackLight(uint8 light_level, uint8 poweroff_save);
void GetBackLight();

void SetBuzzerTime(uint8 t);

void CalibrateTouchPane(void);

void SetScreen(uint16 screen_id);
void GetScreen(void);

void SetRTC(uint8 year, uint8 mon, uint8 date, uint8 hour, uint8 min, uint8 sec);
void GetRTC(void);

void SetTextValue(uint16 addr, char *str);
void SetTextValueLen(uint16 addr, char *str, uint8_t len);
void GetTextValue(uint16 addr, uint8 len);

void SetTextFontColor(uint16 vpAddr, uint16 color);


#endif
