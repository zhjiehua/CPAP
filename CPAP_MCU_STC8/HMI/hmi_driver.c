#include "hmi_driver.h"
#include "uart.h"
#include "misc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TX_8(P1) SEND_DATA((P1)&0xFF)  //发送单个字节
#define TX_8N(P,N) SendNU8((uint8 *)P,N)  //发送N个字节
#define TX_16(P1) TX_8((P1)>>8);TX_8(P1)  //发送16位整数
#define TX_16N(P,N) SendNU16((uint16 *)P,N)  //发送N个16位整数
#define TX_32(P1) TX_16((P1)>>16);TX_16((P1)&0xFFFF)  //发送32位整数

uint8_t sendBuffer[50];
uint8_t sbIndex = 0;

#if(CRC16_ENABLE)

static uint16 _crc16 = 0xffff;
void AddCRC16(uint8 *buffer,uint16 n,uint16 *pcrc)
{
	uint16 i,j,carry_flag,a;

	for (i=0; i<n; i++)
	{
		*pcrc=*pcrc^buffer[i];
		for (j=0; j<8; j++)
		{
			a=*pcrc;
			carry_flag=a&0x0001;
			*pcrc=*pcrc>>1;
			if (carry_flag==1)
				*pcrc=*pcrc^0xa001;
		}
	}
}

uint16 CheckCRC16(uint8 *buffer,uint16 n)
{
	uint16 crc0 = 0x0;
	uint16 crc1 = 0xffff;

	if(n>=2)
	{
		crc0 = ((buffer[n-2]<<8)|buffer[n-1]);
		AddCRC16(buffer,n-2,&crc1);
	}

	return (crc0==crc1);
}

void SEND_DATA(uint8 c)
{
	AddCRC16(&c,1,&_crc16);

	sendBuffer[sbIndex++] = c;
}

void BEGIN_CMD()
{
	sbIndex = 0;

	//TX_16(0XA55A);
	sendBuffer[sbIndex++] = (char)(CMD_HEAD>>8);  //CRC校验不包括指令帧头和指令长度
	sendBuffer[sbIndex++] = (char)CMD_HEAD;

	sbIndex++; //留出长度字节空间

	_crc16 = 0XFFFF;//开始计算CRC16
}

void END_CMD()
{
	uint8_t i;
	uint16 crc16 = _crc16;
	//TX_16(crc16);//发送CRC16
	TX_8(crc16); //高低字节调换发送
	TX_8(crc16>>8);

	sendBuffer[2] = sbIndex - 3;

	for(i=0;i<sbIndex;i++)
	{
		Uart2Send(sendBuffer[i]);
	}
}

#else//NO CRC16   //========================================================================================

//#define SEND_DATA(P) SendChar(P)
//#define BEGIN_CMD() TX_16(0XA55A)
//#define END_CMD() my_serialport->write(sendData)

void SEND_DATA(uint8 c)
{
	sendBuffer[sbIndex++] = c;
}

void BEGIN_CMD()
{
	sbIndex = 0;

	TX_16(LCD_CMD_HEAD);

	sbIndex++; //留出长度字节空间
}

void END_CMD()
{
	uint8_t i;

	sendBuffer[2] = sbIndex - 3;

	for(i=0;i<sbIndex;i++)
	{
		Uart2Send(sendBuffer[i]);
	}
}

#endif

void SendStrings(uint8 *str)
{
	while(*str)
	{
		TX_8((char)(*str));
		str++;
	}
}

void SendNU8(uint8 *pData,uint16 nDataLen)
{
	uint16 i = 0;
	for (;i<nDataLen;++i)
	{
		TX_8(pData[i]);
	}
}

void SendNU16(uint16 *pData,uint16 nDataLen)
{
	uint16 i = 0;
	for (;i<nDataLen;++i)
	{
		TX_16(pData[i]);
	}
}

/*****************************************************************************************
*
*
*
******************************************************************************************/

void ResetDevice()
{
	BEGIN_CMD();
	TX_8(0x80);
	TX_8(0xEE);
	TX_8(0x5A);
	TX_8(0xA5);
	END_CMD();
}

void GetVersion()
{
	BEGIN_CMD();
	TX_8(0x81);
	TX_8(0x00);
	TX_8(0x01);
	END_CMD();
}

void SetBackLight(uint8 light_level, uint8 poweroff_save)
{
	BEGIN_CMD();
	TX_8(0x80);
	TX_8(0x01);
	if(poweroff_save)
		TX_8(0x80 | (light_level & 0x3F));
	else
		TX_8(light_level & 0x3F);
	END_CMD();
}

void GetBackLight()
{
	BEGIN_CMD();
	TX_8(0x81);
	TX_8(0x01);
	TX_8(0x01);
	END_CMD();
}

void SetBuzzerTime(uint8 t) //单位10ms
{
	BEGIN_CMD();
	TX_8(0x80);
	TX_8(0x02);
	TX_8(t);
	END_CMD();
}

void CalibrateTouchPane(void)
{
	BEGIN_CMD();
	TX_8(0x80);
	TX_8(0xEA);
	TX_8(0x5A);
	END_CMD();
}

void GetScreen(void)
{
	BEGIN_CMD();
	TX_8(0x81);
	TX_8(0x03);
	TX_8(0x01);
	END_CMD();
}

void SetScreen(uint16 screen_id) 
{
	BEGIN_CMD();
	TX_8(0x80);
	TX_8(0x03);
	TX_16(screen_id);
	END_CMD();
}

void SetRTC(uint8 year, uint8 mon, uint8 date, uint8 hour, uint8 min, uint8 sec)
{
	BEGIN_CMD();
	TX_8(0x80);
	TX_8(0x1F);
	TX_8(0x5A);
	TX_8(Int2BCD(year));
	TX_8(Int2BCD(mon));
	TX_8(Int2BCD(date));
	TX_8(0x00);
	TX_8(Int2BCD(hour));
	TX_8(Int2BCD(min));
	TX_8(Int2BCD(sec));
	END_CMD();
}

void GetRTC(void)
{
	BEGIN_CMD();
	TX_8(0x81);
	TX_8(0x20);
	TX_8(0x07);
	END_CMD();
}

void SetTextValue(uint16 addr, char *str)
{
	BEGIN_CMD();
	TX_8(0x82);
	TX_8(addr >> 8);
	TX_8(addr);
	while (*str)
	{
		TX_8(*str);
		str++;
	}
	//TX_16(0xFFFF);//文本尾
	END_CMD();
}

void SetTextValueLen(uint16 addr, char *str, uint8_t len)
{
	BEGIN_CMD();
	TX_8(0x82);
	TX_8(addr >> 8);
	TX_8(addr);
	while (len--)
	{
		TX_8(*str);
		str++;
	}
	TX_16(0xFFFF);//文本尾
	END_CMD();
}

void GetTextValue(uint16 addr, uint8 len)
{
	BEGIN_CMD();
	TX_8(0x83);
	TX_8(addr >> 8);
	TX_8(addr);
	TX_8(len);
	END_CMD();
}

void SetTextFontColor(uint16 vpAddr, uint16 color)
{
	uint16 colorDesAddr = vpAddr + 0x03;
	BEGIN_CMD();
	TX_8(0x82);
	TX_8(colorDesAddr >> 8);
	TX_8(colorDesAddr);
	TX_8(color >> 8);
	TX_8(color);
	END_CMD();
}

#ifdef __cplusplus
}
#endif