
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>	
#include "baseband.h"
#include "gateway.h"
#include "common.h"
#include "crc.h"

#define MAX_POWER 47
//#define NO_CRC_CHECK
//#define DEBUG

uint8_t gateway(uint8_t fd, uint8_t *ret,uint8_t *state,uint8_t *message,uint16_t *msg_ptr)
{
	uint8_t tx_buffer[1],rx_buffer[1],Msg_data[256];
	int rx_length = 0;    
	uint16_t DeltaF=0;
	uint32_t Freq=0;
	uint8_t Power,Msg_len,crc,local_crc;
		memset( Msg_data, 0, 256);
	*state=GET_DELTAF;

	
	if (fd != -1)
		{
			
			while(*state!=SEND_ACK)
			{
				// Read up to 255 characters from the port if they are there
				if (*state!=SEND_ACK)
				{
					
					rx_length = read(fd, (void*)rx_buffer, 1);		//Filestream, buffer to store in, number of bytes to read (max)
					
				}
				if ((mytimeout==1)&&(msg_ptr>0))//if timeout after received at least one char	
				{

					mytimeout=0;
					*state=SEND_ACK;
					*ret|=MSG_TIMEOUT;	
						
				}


				if ((rx_length > 0) || (*state==SEND_ACK)) //no need to wait for a char to send ACK-> must be done immediately
				{
					switch (*state)
					{
							case GET_DELTAF:
							
							DeltaF=rx_buffer[0]*256;
							message[(*msg_ptr)++]=rx_buffer[0];
							rx_length = read(fd, (void*)rx_buffer, 1);
							message[(*msg_ptr)++]=rx_buffer[0];
							
							if (rx_length > 0)
							{
								DeltaF+=rx_buffer[0];
								#ifdef DEBUG								
								printf("DeltaF received %d\n",DeltaF);
								#endif
								*state=GET_FREQ;
							}
							else
							{
								#ifdef DEBUG								
								printf("DeltaF error \n");
								#endif
								*state=GET_START_BYTE;
							}
						break;
						 case GET_FREQ:
							
							Freq=rx_buffer[0];
							#ifdef DEBUG								
								printf("FREQ 1st byte received %02X\n",rx_buffer[0]);
							#endif
							message[(*msg_ptr)++]=rx_buffer[0];
							rx_length = read(fd, (void*)rx_buffer, 1);
							message[(*msg_ptr)++]=rx_buffer[0];
							
							if (rx_length > 0)
							{
								Freq=Freq*256+rx_buffer[0];
								#ifdef DEBUG								
								printf("FREQ 2nd byte received %02X\n",rx_buffer[0]);
								#endif
								
							}
							else
							{
								#ifdef DEBUG								
								printf("DeltaF error \n");
								#endif
								*state=GET_START_BYTE;
							}
							rx_length = read(fd, (void*)rx_buffer, 1);
							message[(*msg_ptr)++]=rx_buffer[0];
							
							if (rx_length > 0)
							{
								Freq=Freq*256+rx_buffer[0];
								#ifdef DEBUG								
								printf("FREQ 3rd byte received %02X\n",rx_buffer[0]);
								#endif
								
							}
							else
							{
								#ifdef DEBUG								
								printf("FREQ error \n");
								#endif
								*state=GET_START_BYTE;
							}
							rx_length = read(fd, (void*)rx_buffer, 1);
							message[(*msg_ptr)++]=rx_buffer[0];
							
							if (rx_length > 0)
							{
								Freq=Freq*256+rx_buffer[0];
								#ifdef DEBUG								
								printf("FREQ received %d\n",Freq);
								#endif
								*state=GET_POWER;
							}
							else
							{
								#ifdef DEBUG								
								printf("FREQ error \n");
								#endif
								*state=GET_START_BYTE;
							}

						break;
						case GET_POWER:
						
							if (rx_buffer[0]>MAX_POWER)
							{
								#ifdef DEBUG
								printf("0x%02X Wrong power byte byte received \n", rx_buffer[0]);
								#endif
								*state=SEND_ACK;
								*ret|=SIGNAL_STRENGTH_ERROR;  
							}
							else
							{
								Power=rx_buffer[0];
								#ifdef DEBUG
								printf("Power byte received: %d \n",Power);
								#endif
								*state=GET_MSG_LEN;
								message[(*msg_ptr)++]=rx_buffer[0];
								

							}
						break;
						case GET_MSG_LEN:
						
							
								Msg_len=rx_buffer[0];
								#ifdef DEBUG
								printf("Message len byte received: %d \n",Msg_len);
								#endif
								*state=GET_MSG;
								message[(*msg_ptr)++]=rx_buffer[0];
						break;
						case GET_MSG:
								#ifdef DEBUG
								printf("receiving Message\n");
								#endif
								
								for (uint8_t i=0;i<Msg_len;i++)
								{
									
									if (rx_length > 0)
									{
										Msg_data[i]=rx_buffer[0];
										message[(*msg_ptr)++]=rx_buffer[0];
									}
									if (i<Msg_len-1)rx_length = read(fd, (void*)rx_buffer, 1);
								}
								#ifdef DEBUG
								printf("Message received \n");
								for (uint8_t i=0;i<Msg_len;i++)
								{
									printf("%02X ",Msg_data[i]);
								}
								printf("\n");
								#endif
							
								*state=GET_CRC;
						break;
						case GET_CRC:
														
								crc=rx_buffer[0];
								local_crc=getCRC(message,*msg_ptr);
								#ifdef DEBUG
								printf("CRC received: %02X \n",crc);
								printf("local CRC : %02X \n",local_crc);
								
								printf("Complete messsage \n");
								for (uint8_t i=0;i<*msg_ptr;i++)
								{
									printf("%02X ",message[i]);
								}
								printf("\n");
								#endif   
								if (local_crc!=crc)
								{
									*ret|=CRC_ERROR;
								
								}
								#ifndef NO_CRC_CHECK
								else
								#endif
								{
									*ret|=baseband_gen((double) DeltaF,Msg_data,Msg_len,Power,Freq);
								}
								*state=SEND_ACK;
								return 0;
					}//switch		
				} //if rx_length or SEND_ACK					
			}//while						
		}//if fd!=-1						
}
