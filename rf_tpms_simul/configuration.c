
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>	
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "common.h"
#include "crc.h"
#include <sys/reboot.h>

//#define DEBUG
uint8_t tx_buffer[1],rx_buffer[1];

void get_IP(uint8_t fd)
{
	
	
    unsigned char ip_address[15];
    int mysocket,i;
    struct ifreq ifr;

    /*AF_INET - to define network interface IPv4*/
    /*Creating soket for it.*/
    mysocket = socket(AF_INET, SOCK_DGRAM, 0);

    /*AF_INET - to define IPv4 Address type.*/
    ifr.ifr_addr.sa_family = AF_INET;

    /*eth0 - define the ifr_name - port name
    where network attached.*/
    memcpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

    /*Accessing network interface information by
    passing address using ioctl.*/
    ioctl(mysocket, SIOCGIFADDR, &ifr);
    /*closing fd*/
    close(mysocket);

    /*Extract IP Address*/
    strcpy(ip_address, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));
	for(i=0;ip_address[i]!=0;i++)
	{
		
		tx_buffer[0]=ip_address[i];	
		write(fd, (void*)tx_buffer, 1);  
		if (i>=15)break;
		
	}
	#ifdef DEBUG
    printf("System IP Address is: %s\n", ip_address);
	#endif
}

void myreboot()
{
	#ifdef DEBUG
	printf("reboot\n");
	#endif
	sync();
	reboot(RB_AUTOBOOT);
	while(1);
}

void myshutdown()
{
	#ifdef DEBUG
		printf("shutdown\n");
	#endif
	sync();
	reboot(RB_POWER_OFF );
	while(1);
}

//#define DEBUG

uint8_t config(uint8_t fd, uint8_t *ret,uint8_t *state,uint8_t *message,uint16_t *msg_ptr)
{
	uint8_t Msg_data[256];
	int rx_length = 0;    
	uint16_t DeltaF;
	uint8_t Power,Msg_len,crc,local_crc;
	memset( Msg_data, 0, 256);
	*state=GET_MSG_LEN;

	
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
									#ifdef DEBUG
									printf("Wrong CRC; command denied\n");
									#endif
								}
								else
								{
									
									tx_buffer[0]=*ret;	
									if ( strcmp( Msg_data, "reboot" ) == 0 )
									{
										rx_length = write(fd, (void*)tx_buffer, 1); 
										myreboot();
									}
									if ( strcmp( Msg_data, "GetIP" ) == 0 )
									{
										get_IP(fd);
									}
									if ( strcmp( Msg_data, "shutdown" ) == 0 )
									{
										rx_length = write(fd, (void*)tx_buffer, 1); 
										myshutdown();
									}
								}
								*state=SEND_ACK;
								return 0;
					}//switch		
				} //if rx_length or SEND_ACK					
			}//while						
		}//if fd!=-1						
}
