/****************************************************************************************************************************
*
*   文 件 名 ： crc32.c 
*   文件描述 ：  
*   创建日期 ：2019年2月2日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#include "crc32.h"

uint32_t mCrcTable[256];

//位逆序
static uint32_t bit_reverse(uint32_t bitValue, int bw)
{
	int i = 0;
	uint32_t newValue = 0;
	for(i = 0; i < bw; i ++)
	{
		if((bitValue & (1 << i)) != 0)
			newValue |= (1 << (bw - 1 - i));
	}
	
	return newValue;
}

static int initialize_crc32_table()
{	
	uint32_t poly = bit_reverse(0X04C11DB7, 32);
	uint32_t tableEntry;
	uint64_t Index;
	uint32_t Value;

	for(tableEntry = 0; tableEntry < 256; tableEntry ++)
	{
		Value = tableEntry;
		for(Index = 0; Index < 8; Index++)
		{
			if(Value & 1)
				Value = (Value >> 1) ^ poly;
			else
				Value = Value >>1;
		}
		
		mCrcTable[tableEntry] = Value;
	}

	return 0;
}

uint32_t crc32(void * data, uint64_t len)
{
	if(!data)
		return 0;
	
	initialize_crc32_table();
	
	uint8_t * Ptr = (uint8_t *)data;
	uint32_t crc = 0xFFFFFFFF;;
	int i = 0;
	for(i = 0; i < len; i ++)
	{
		crc = (crc >> 8) ^ mCrcTable[ (uint8_t)crc ^ *Ptr];
		Ptr++;
	}
	
	uint32_t crcOut = crc ^ 0xFFFFFFFF;
	return crcOut;
}


