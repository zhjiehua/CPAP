#include "common.h"
#include "uart.h"
#include "io.h"
#include "cmd_process.h"
#include "stepmotor.h"
#include "misc.h"

void Sort32(uint32_t *pBuffer, uint8_t count)
{
	uint8_t i, j;
	uint32_t temp;
	for(i=0;i<count-1;i++)
	{
		for(j=i+1;j<count;j++)
		{
			if(pBuffer[i] > pBuffer[j])
			{
				temp = pBuffer[j];
				pBuffer[j] = pBuffer[i];
				pBuffer[i] = temp;
			}
		}
	}
}

uint32_t GetAverage32(uint32_t *pBuffer, uint8_t count, uint8_t discardCount)
{
	uint8_t i;
	uint32_t average = 0;

	for(i=discardCount;i<(count-discardCount);i++)
		average += pBuffer[i];

	average /= (count-2*discardCount);
	return average;
}

void Sort16(uint16_t *pBuffer, uint8_t count)
{
	uint8_t i, j;
	uint16_t temp;
	for(i=0;i<count-1;i++)
	{
		for(j=i+1;j<count;j++)
		{
			if(pBuffer[i] > pBuffer[j])
			{
				temp = pBuffer[j];
				pBuffer[j] = pBuffer[i];
				pBuffer[i] = temp;
			}
		}
	}
}

uint16_t GetAverage16(uint16_t *pBuffer, uint8_t count, uint8_t discardCount)
{
	uint8_t i;
	uint32_t average = 0;

	for(i=discardCount;i<(count-discardCount);i++)
		average += pBuffer[i];

	average /= (count-2*discardCount);
	return (uint16_t)average;
}

void Float2String(float f, char *integer, char *decimal)
{
	*decimal = ((int)(f*10.0))%10 + 0x30;
	*(integer+1) = ((int)(f)%10) + 0x30;
	*(integer) = ((int)(f))/10%10 + 0x30;
}

void Int2String(int i, char *integer)
{
	*(integer+1) = (i%10) + 0x30;
	*(integer) = i/10%10 + 0x30;
}

uint16_t Int2BCD(int m)
{
    uint16_t r=0,n=1;
    int a;
    while(m)
    {
        a=m %10;
        m=m/10;
        r=r+n*a;
        n=n<<4;
    } 
    return r;
}

int BCD2Int(uint16_t m)
{
   int a=0,b=1;
   uint16_t n;
   while(m)
   {
      n=0xF&m;
      m=m>>4;
      a=a+b*n;
      b=b*10;
   }
   return a;
}