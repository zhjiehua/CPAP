#include "tm770x.h" 
#include "io.h"
#include "uart.h"

void TM770X_SyncSPI(uint8_t chip)
{
    TM770X_WriteRegister(chip, 0xFF);
    TM770X_WriteRegister(chip, 0xFF);
    TM770X_WriteRegister(chip, 0xFF);
    TM770X_WriteRegister(chip, 0xFF);        
}

void TM770X_WriteRegister(uint8_t chip, uint8_t ByteData)
{
    uint8_t i;
    switch(chip)
    {
        case 0:
            A_CS1 = 0;
        break;
        case 1:
            A_CS2 = 0;
        break;
        case 2:
            A_CS3 = 0;
        break;
        default:
            A_CS1 = 0;
        break;
    }

    for(i=0;i<8;i++)
    {
        A_SCLK = 0;

        if(ByteData&0x80)
            A_DATA = 1;
        else
            A_DATA = 0;

        ByteData <<= 1;

        A_SCLK = 1;
    }
    switch(chip)
    {
        case 0:
            A_CS1 = 1;
        break;
        case 1:
            A_CS2 = 1;
        break;
        case 2:
            A_CS3 = 1;
        break;
        default:
            A_CS1 = 1;
        break;
    }

    A_DATA = 1;  //最后一定要拉高
}

uint8_t TM770X_ReadRegister(uint8_t chip, uint8_t *pReadData, uint8_t RegLen)
{
    uint8_t i, j, temp1;
    uint8_t *p = pReadData; 
    uint32_t cnt = 0xFFFFFFFF;
    switch(chip)
    {
        case 0:
            while(A_DRDY1 & cnt)
                cnt--;
        break;
        case 1:
            while(A_DRDY2 & cnt)
                cnt--;
        break;
        case 2:
            while(A_DRDY3 & cnt)
                cnt--;
        break;
        default:
            while(A_DRDY1 & cnt)
                cnt--;
        break;
    }
    
    if(cnt == 0) //超时
    {
        cDebug("TM770X_ReadRegister() 芯片应答超时 ...\r\n");
        cDebug("重新同步SPI接口时序\r\n");
        TM770X_SyncSPI(chip);
        Delay10ms();
        return 1;
    }
    
    switch(chip)
    {
        case 0:
            A_CS1 = 0;
        break;
        case 1:
            A_CS2 = 0;
        break;
        case 2:
            A_CS3 = 0;
        break;
        default:
            A_CS1 = 0;
        break;
    }

    for(i=0;i<RegLen;i++)
    {
        for(j=0;j<8;j++)
        {
            A_SCLK = 0;
            A_SCLK = 1;

            *p <<= 1;

            if(A_DATA)
                *p += 0x01;
        }

        p++;
    }
    switch(chip)
    {
        case 0:
            A_CS1 = 1;
        break;
        case 1:
            A_CS2 = 1;
        break;
        case 2:
            A_CS3 = 1;
        break;
        default:
            A_CS1 = 1;
        break;
    }
    
//    cnt = 2550;
//    switch(chip)
//    {
//        case 0:
//            while(A_DRDY1 & cnt)
//                cnt--;
//        break;
//        case 1:
//            while(A_DRDY2 & cnt)
//                cnt--;
//        break;
//        case 2:
//            while(A_DRDY3 & cnt)
//                cnt--;
//        break;
//        default:
//            while(A_DRDY1 & cnt)
//                cnt--;
//        break;
//    }
//    if(cnt == 0)
//        return 1;
       
    return 0;    
}

uint8_t TM770X_ReadData(uint8_t ch, uint8_t *pReadData)
{
    switch(ch)
    {
        case 0:
            TM770X_WriteRegister(0, TM7706_REG_DATA|READ_REG|CHANNAL_2);
            return (TM770X_ReadRegister(0, pReadData, 2));
        break;
        case 1:
            TM770X_WriteRegister(0, TM7706_REG_DATA|READ_REG|CHANNAL_1);
            return (TM770X_ReadRegister(0, pReadData, 2));
        break;
        case 2:
            TM770X_WriteRegister(1, TM7707_REG_DATA|READ_REG|CHANNAL_1);
            return (TM770X_ReadRegister(1, pReadData, 3));
        break;
        case 3:
            TM770X_WriteRegister(2, TM7707_REG_DATA|READ_REG|CHANNAL_1);
            return (TM770X_ReadRegister(2, pReadData, 3));
        break;
        case 4:
            TM770X_WriteRegister(2, TM7707_REG_DATA|READ_REG|CHANNAL_2);
            return (TM770X_ReadRegister(2, pReadData, 3));
        break;
        default:
            return 1;
        break;    
    }
}

uint8_t TM770X_ReadCalibData(uint8_t ch, uint8_t calibType, uint8_t *pReadData)
{
    if(calibType == 0) //0标度
    {
        switch(ch)
        {
            case 0:
                TM770X_WriteRegister(0, TM7706_REG_OFFSET|READ_REG|CHANNAL_2);
                return (TM770X_ReadRegister(0, pReadData, 2));
            break;
            case 1:
                TM770X_WriteRegister(0, TM7706_REG_OFFSET|READ_REG|CHANNAL_1);
                return (TM770X_ReadRegister(0, pReadData, 2));
            break;
            case 2:
                TM770X_WriteRegister(1, TM7707_REG_CALIB_Z|READ_REG|CHANNAL_1);
                return (TM770X_ReadRegister(1, pReadData, 3));
            break;
            case 3:
                TM770X_WriteRegister(2, TM7707_REG_CALIB_Z|READ_REG|CHANNAL_1);
                return (TM770X_ReadRegister(2, pReadData, 3));
            break;
            case 4:
                TM770X_WriteRegister(2, TM7707_REG_CALIB_Z|READ_REG|CHANNAL_2);
                return (TM770X_ReadRegister(2, pReadData, 3));
            break;
            default:
                return 1;
            break;    
        }
    }
    else //满标度
    {
        switch(ch)
        {
            case 0:
                TM770X_WriteRegister(0, TM7706_REG_GAIN|READ_REG|CHANNAL_2);
                return (TM770X_ReadRegister(0, pReadData, 3));
            break;
            case 1:
                TM770X_WriteRegister(0, TM7706_REG_GAIN|READ_REG|CHANNAL_1);
                return (TM770X_ReadRegister(0, pReadData, 3));
            break;
            case 2:
                TM770X_WriteRegister(1, TM7707_REG_CALIB_F|READ_REG|CHANNAL_1);
                return (TM770X_ReadRegister(1, pReadData, 3));
            break;
            case 3:
                TM770X_WriteRegister(2, TM7707_REG_CALIB_F|READ_REG|CHANNAL_1);
                return (TM770X_ReadRegister(2, pReadData, 3));
            break;
            case 4:
                TM770X_WriteRegister(2, TM7707_REG_CALIB_F|READ_REG|CHANNAL_2);
                return (TM770X_ReadRegister(2, pReadData, 3));
            break;
            default:
                return 1;
            break;    
        }
    }
}

void TM770X_Init(uint8_t chip)
{
    uint32_t cnt;
//    A_RST = 1;
//    Delay10ms();
//    A_RST = 0;
//    Delay10ms();
//    A_RST = 1;
//    Delay10ms();

    if(chip & CHIP0_TM7706)
    {
        //TM7706，2个通道，接2个氧传感器
    //    TM770X_SyncSPI(0);		/* 同步SPI接口时序 */
    //    Delay10ms();
        TM770X_WriteRegister(0, TM7706_REG_TEST);
        TM770X_WriteRegister(0, 0x01); //使用内部基准
        TM770X_WriteRegister(0, TM7706_REG_CLK);
        TM770X_WriteRegister(0, 0x14|0x02); //使用1MHz时钟需要将CLK位置1，250Hz更新频率
        TM770X_WriteRegister(0, TM7706_REG_SETUP|CHANNAL_1);
        TM770X_WriteRegister(0, MODE_CALIB_SELF|GAIN_32|POLARITY_U|BUFFER_USED); //双极性，增益128，进入自校准   BUFFER_USED    BUFFER_NONE
        cnt = 0;
        while(A_DRDY1 && cnt < 60000) cnt++;
        //while(A_DRDY1);
        TM770X_WriteRegister(0, TM7706_REG_SETUP|CHANNAL_2);
        TM770X_WriteRegister(0, MODE_CALIB_SELF|GAIN_32|POLARITY_U|BUFFER_USED); //双极性，增益128，进入自校准
        cnt = 0;
        while(A_DRDY1 && cnt < 60000) cnt++;
        //while(A_DRDY1);
    }

    if(chip & CHIP1_TM7707)
    {
        //TM7707，1个通道，接1个温度传感器
    //    TM770X_SyncSPI(1);		/* 同步SPI接口时序 */
    //    Delay10ms();
        TM770X_WriteRegister(1, TM7707_REG_FILTER_L);//向通信寄存器写数据，选择通道1 作为有效通道,将下一个操作设为对滤波低寄存器进行写操作。
        TM770X_WriteRegister(1, 0x04); //向滤波低寄存器写入0X00。
        TM770X_WriteRegister(1, TM7707_REG_FILTER_H);//向通信寄存器写数据，选择通道1 作为有效通道,将下一个操作设为对滤波高寄存器进行写操作。
        TM770X_WriteRegister(1, 0x31); //向滤波高寄存器写入0X0F。
        //更新率 = (fCLKIN/128)/filter = 1000000/128/filter = 7812.5/filter，若更新率为250Hz，则filter = 0x0020
        TM770X_WriteRegister(1, TM7707_REG_SETUP|CHANNAL_1);//向通信寄存器写数据,选择通道1 作为有效通道，将下一个操作设为对设置寄存器进行写操作。
        TM770X_WriteRegister(1, MODE_CALIB_SELF|GAIN_128|POLARITY_U|BUFFER_USED); //对设置寄存器写操作，选择16倍增益，无BUF缓冲器,双极性，进入自校准。
        cnt = 0;
        while(A_DRDY2 && cnt < 120000) cnt++;
        //while(A_DRDY2);
    }

    if(chip & CHIP2_TM7707)
    {
        //TM7707，2个通道，接2个温度传感器
    //    TM770X_SyncSPI(2);		/* 同步SPI接口时序 */
    //    Delay10ms();
        TM770X_WriteRegister(2, TM7707_REG_FILTER_L);//向通信寄存器写数据，选择通道1 作为有效通道,将下一个操作设为对滤波低寄存器进行写操作。
        TM770X_WriteRegister(2, 0x00); //向滤波低寄存器写入0X00。
        TM770X_WriteRegister(2, TM7707_REG_FILTER_H);//向通信寄存器写数据，选择通道1 作为有效通道,将下一个操作设为对滤波高寄存器进行写操作。
        TM770X_WriteRegister(2, 0x31); //向滤波高寄存器写入0X0F。
        TM770X_WriteRegister(2, TM7707_REG_SETUP|CHANNAL_1);//向通信寄存器写数据,选择通道1 作为有效通道，将下一个操作设为对设置寄存器进行写操作。
        TM770X_WriteRegister(2, MODE_CALIB_SELF|GAIN_128|POLARITY_U|BUFFER_USED); //对设置寄存器写操作，选择16倍增益，无BUF缓冲器,双极性，进入自校准。
        cnt = 0;
        while(A_DRDY3 && cnt < 120000) cnt++;
        //while(A_DRDY3);
        TM770X_WriteRegister(2, TM7707_REG_SETUP|CHANNAL_2);//向通信寄存器写数据,选择通道1 作为有效通道，将下一个操作设为对设置寄存器进行写操作。
        TM770X_WriteRegister(2, MODE_CALIB_SELF|GAIN_128|POLARITY_U|BUFFER_USED); //对设置寄存器写操作，选择16倍增益，无BUF缓冲器,双极性，进入自校准。
        cnt = 0;
        while(A_DRDY3 && cnt < 120000) cnt++;
        //while(A_DRDY3);
    }
}
