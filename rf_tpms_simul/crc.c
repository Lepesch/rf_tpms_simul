#include "crc.h"

uint8_t getCRC(uint8_t message[],uint16_t length)
{

	uint8_t CRC7_POLY=0x2F;
	uint8_t i,j,crc=0xAA;
	for(i=0;i<length;i++)
	{
			//printf("message [%d]=%02X\n",i,message[i]);
			crc^=message[i];
			for(j=0;j<8;j++)
					{
						if((crc & 0x80)!=0){ crc=((crc<<1)^CRC7_POLY); }
						else{
						crc<<=1;}
					}
	}
	return crc;

}