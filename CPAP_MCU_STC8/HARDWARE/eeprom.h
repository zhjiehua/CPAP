/*
	stc89c52的eeprom大小有4KByte

		地址			大小		 扇区
	0x2000~0x21FF      512Byte   	sector1
	0x2200~0x23FF      512Byte   	sector2
	0x2400~0x25FF      512Byte   	sector3
	0x2600~0x27FF      512Byte   	sector4
	0x2800~0x29FF      512Byte   	sector5
	0x2A00~0x2BFF      512Byte   	sector6
	0x2C00~0x2DFF      512Byte   	sector7
	0x2E00~0x2FFF      512Byte   	sector8
*/

#ifndef __EEPROM_H__
#define __EEPROM_H__

//typedef unsigned char BYTE;
//typedef unsigned int WORD;
//
///*Declare SFR associated with the IAP */
////sfr IAP_DATA    =   0xE2;           //Flash data register
////sfr IAP_ADDRH   =   0xE3;           //Flash address HIGH
////sfr IAP_ADDRL   =   0xE4;           //Flash address LOW
////sfr IAP_CMD     =   0xE5;           //Flash command register
////sfr IAP_TRIG    =   0xE6;           //Flash command trigger
////sfr IAP_CONTR   =   0xE7;           //Flash control register
//
///*Define ISP/IAP/EEPROM command*/
//#define CMD_IDLE    0               //Stand-By
//#define CMD_READ    1               //Byte-Read
//#define CMD_PROGRAM 2               //Byte-Program
//#define CMD_ERASE   3               //Sector-Erase
//
///*Define ISP/IAP/EEPROM operation const for IAP_CONTR*/
////#define ENABLE_IAP 0x80           //if SYSCLK<40MHz
////#define ENABLE_IAP   0x81           //if SYSCLK<20MHz
////#define ENABLE_IAP x82            //if SYSCLK<10MHz
////#define ENABLE_IAP 0x83           //if SYSCLK<5MHz
//
////Start address for STC89C52xx EEPROM
//#define IAP_ADDRESS 0x2000
//
//void Delay(BYTE n);
//void IapIdle();
//BYTE IapReadByte(WORD addr);
//void IapProgramByte(WORD addr, BYTE dat);
//void IapEraseSector(WORD addr);


//sfr     IAP_DATA    =   0xC2;
//sfr     IAP_ADDRH   =   0xC3;
//sfr     IAP_ADDRL   =   0xC4;
//sfr     IAP_CMD     =   0xC5;
//sfr     IAP_TRIG    =   0xC6;
//sfr     IAP_CONTR   =   0xC7;

#define WT_30M          0x80
#define WT_24M          0x81
#define WT_20M          0x82
#define WT_12M          0x83
#define WT_6M           0x84
#define WT_3M           0x85
#define WT_2M           0x86
#define WT_1M           0x87

#define ENABLE_IAP      WT_30M

char IapRead(int addr);
void IapProgram(int addr, char dat);
void IapErase(int addr);

void EEPROM_WriteBytes(uint16_t WriteAddr, uint8_t *DataToWrite, uint8_t DataLen);
void EEPROM_ReadBytes(uint16_t ReadAddr, uint8_t *DataToRead, uint8_t DataLen);

#endif