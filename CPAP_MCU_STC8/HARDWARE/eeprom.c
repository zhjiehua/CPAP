/*------------------------------------------------------------------*/
/* --- STC MCU Limited ---------------------------------------------*/
/* --- STC89-90xx Series MCU ISP/IAP/EEPROM Demo -------------------*/
/* --- Mobile: (86)13922805190 -------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ---------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966----------------------*/
/* --- Web: www.STCMCU.com -----------------------------------------*/
/* --- Web: www.GXWMCU.com -----------------------------------------*/
/* If you want to use the program or the program referenced in the  */
/* article, please specify in which data and procedures from STC    */
/*------------------------------------------------------------------*/

#include "common.h"
#include "eeprom.h"
#include "timer.h"

///*----------------------------
//Software delay function
//----------------------------*/
//void Delay(BYTE n)
//{
//    WORD x;
//
//    while (n--)
//    {
//        x = 0;
//        while (++x);
//    }
//}
//
///*----------------------------
//Disable ISP/IAP/EEPROM function
//Make MCU in a safe state
//----------------------------*/
//void IapIdle()
//{
//    IAP_CONTR = 0;                  //Close IAP function
//    IAP_CMD = 0;                    //Clear command to standby
//    IAP_TRIG = 0;                   //Clear trigger register
//    IAP_ADDRH = 0x80;               //Data ptr point to non-EEPROM area
//    IAP_ADDRL = 0;                  //Clear IAP address to prevent misuse
//}
//
///*----------------------------
//Read one byte from ISP/IAP/EEPROM area
//Input: addr (ISP/IAP/EEPROM address)
//Output:Flash data
//----------------------------*/
//BYTE IapReadByte(WORD addr)
//{
//    BYTE dat;                       //Data buffer
//
//    IAP_CONTR = ENABLE_IAP;         //Open IAP function, and set wait time
//    IAP_CMD = CMD_READ;             //Set ISP/IAP/EEPROM READ command
//    IAP_ADDRL = addr;               //Set ISP/IAP/EEPROM address low
//    IAP_ADDRH = addr >> 8;          //Set ISP/IAP/EEPROM address high
//    IAP_TRIG = 0x46;                //Send trigger command1 (0x46)
//    IAP_TRIG = 0xb9;                //Send trigger command2 (0xb9)
//    _nop_();                        //MCU will hold here until ISP/IAP/EEPROM operation complete
//    dat = IAP_DATA;                 //Read ISP/IAP/EEPROM data
//    IapIdle();                      //Close ISP/IAP/EEPROM function
//
//    return dat;                     //Return Flash data
//}
//
///*----------------------------
//Program one byte to ISP/IAP/EEPROM area
//Input: addr (ISP/IAP/EEPROM address)
//       dat (ISP/IAP/EEPROM data)
//Output:-
//----------------------------*/
//void IapProgramByte(WORD addr, BYTE dat)
//{
//    IAP_CONTR = ENABLE_IAP;         //Open IAP function, and set wait time
//    IAP_CMD = CMD_PROGRAM;          //Set ISP/IAP/EEPROM PROGRAM command
//    IAP_ADDRL = addr;               //Set ISP/IAP/EEPROM address low
//    IAP_ADDRH = addr >> 8;          //Set ISP/IAP/EEPROM address high
//    IAP_DATA = dat;                 //Write ISP/IAP/EEPROM data
//    IAP_TRIG = 0x46;                //Send trigger command1 (0x46)
//    IAP_TRIG = 0xb9;                //Send trigger command2 (0xb9)
//    _nop_();                        //MCU will hold here until ISP/IAP/EEPROM operation complete
//    IapIdle();
//}
//
///*----------------------------
//Erase one sector area
//Input: addr (ISP/IAP/EEPROM address)
//Output:-
//----------------------------*/
//void IapEraseSector(WORD addr)
//{
//    IAP_CONTR = ENABLE_IAP;         //Open IAP function, and set wait time
//    IAP_CMD = CMD_ERASE;            //Set ISP/IAP/EEPROM ERASE command
//    IAP_ADDRL = addr;               //Set ISP/IAP/EEPROM address low
//    IAP_ADDRH = addr >> 8;          //Set ISP/IAP/EEPROM address high
//    IAP_TRIG = 0x46;                //Send trigger command1 (0x46)
//    IAP_TRIG = 0xb9;                //Send trigger command2 (0xb9)
//    _nop_();                        //MCU will hold here until ISP/IAP/EEPROM operation complete
//    IapIdle();
//}
//
//
///*
//	//下面是测试部分程序
//    P1 = 0xfe;                      //1111,1110 System Reset OK
//    Delay(10);                      //Delay
//    IapEraseSector(IAP_ADDRESS);    //Erase current sector
//    for (i=0; i<512; i++)           //Check whether all sector data is FF
//    {
//        if (IapReadByte(IAP_ADDRESS+i) != 0xff)
//            goto Error;             //If error, break
//    }
//    P1 = 0xfc;                      //1111,1100 Erase successful
//    Delay(10);                      //Delay
//    for (i=0; i<512; i++)           //Program 512 bytes data into data flash
//    {
//        IapProgramByte(IAP_ADDRESS+i, (BYTE)i);
//    }
//    P1 = 0xf8;                      //1111,1000 Program successful
//    Delay(10);                      //Delay
//    for (i=0; i<512; i++)           //Verify 512 bytes data
//    {
//        if (IapReadByte(IAP_ADDRESS+i) != (BYTE)i)
//            goto Error;             //If error, break
//    }
//    P1 = 0xf0;                      //1111,0000 Verify successful
//    while (1);
//Error:
//    P1 &= 0x7f;                     //0xxx,xxxx IAP operation fail
//    while (1);
//*/


void IapIdle()
{
    IAP_CONTR = 0;                              //关闭IAP功能
    IAP_CMD = 0;                                //清除命令寄存器
    IAP_TRIG = 0;                               //清除触发寄存器
    IAP_ADDRH = 0x80;                           //将地址设置到非IAP区域
    IAP_ADDRL = 0;
}

char IapRead(int addr)
{
    char dat;

    IAP_CONTR = ENABLE_IAP;                     //使能IAP
    IAP_CMD = 1;                                //设置IAP读命令
    IAP_ADDRL = addr;                           //设置IAP低地址
    IAP_ADDRH = addr >> 8;                      //设置IAP高地址
    IAP_TRIG = 0x5a;                            //写触发命令(0x5a)
    IAP_TRIG = 0xa5;                            //写触发命令(0xa5)
    _nop_();
    dat = IAP_DATA;                             //读IAP数据
    IapIdle();                                  //关闭IAP功能

    return dat;
}

void IapProgram(int addr, char dat)
{
    IAP_CONTR = ENABLE_IAP;                     //使能IAP
    IAP_CMD = 2;                                //设置IAP写命令
    IAP_ADDRL = addr;                           //设置IAP低地址
    IAP_ADDRH = addr >> 8;                      //设置IAP高地址
    IAP_DATA = dat;                             //写IAP数据
    IAP_TRIG = 0x5a;                            //写触发命令(0x5a)
    IAP_TRIG = 0xa5;                            //写触发命令(0xa5)
    _nop_();
    IapIdle();                                  //关闭IAP功能
}

void IapErase(int addr)
{
    IAP_CONTR = ENABLE_IAP;                     //使能IAP
    IAP_CMD = 3;                                //设置IAP擦除命令
    IAP_ADDRL = addr;                           //设置IAP低地址
    IAP_ADDRH = addr >> 8;                      //设置IAP高地址
    IAP_TRIG = 0x5a;                            //写触发命令(0x5a)
    IAP_TRIG = 0xa5;                            //写触发命令(0xa5)
    _nop_();                                    //
    IapIdle();                                  //关闭IAP功能
}

//这2个函数可用，不知是不是堆栈溢出了的问题
void EEPROM_WriteBytes(uint16_t WriteAddr, uint8_t *DataToWrite, uint8_t DataLen)
{
    uint8_t i;

    for(i=0;i<DataLen;i++)
        IapProgram(WriteAddr+i, DataToWrite[i]);    												    
}

void EEPROM_ReadBytes(uint16_t ReadAddr, uint8_t *DataToRead, uint8_t DataLen)
{  	
    uint8_t i;

    for(i=0;i<DataLen;i++)
        DataToRead[i] = IapRead(ReadAddr+i);													    
}