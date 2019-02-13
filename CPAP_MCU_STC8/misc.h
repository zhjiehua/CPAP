#ifndef __MISC_H__
#define __MISC_H__

void Sort32(uint32_t *pBuffer, uint8_t count);
uint32_t GetAverage32(uint32_t *pBuffer, uint8_t count, uint8_t discardCount);
void Sort16(uint16_t *pBuffer, uint8_t count);
uint16_t GetAverage16(uint16_t *pBuffer, uint8_t count, uint8_t discardCount);

void Float2String(float f, char *integer, char *decimal);
void Int2String(int i, char *integer);

uint16_t Int2BCD(int m);
int BCD2Int(uint16_t m);

#endif
