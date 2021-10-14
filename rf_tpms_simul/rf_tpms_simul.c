#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <inttypes.h>
#include <unistd.h>	
#include <fcntl.h>	
#include <termios.h>
#include "rf_tpms_simul.h"
#include "gateway.h"
#include "common.h"
#include "configuration.h"


#define START_BYTE 0x47
//#define DEBUG
#define MAX_ACTION	10


typedef struct
{
uint8_t (*Action)(uint8_t fd, uint8_t *ret,uint8_t *state,uint8_t *messsage,uint16_t *msg_ptr);//
uint8_t TAG;
char *name;
}
__ACTION__;

__ACTION__  action_table[MAX_ACTION];





void timeout(int signo)
{
//printf("timeout \n");
mytimeout=1;
}

void init_sigaction(void)
{
	struct sigaction tact;
	tact.sa_handler=timeout;
	tact.sa_flags=0;
	sigemptyset(&tact.sa_mask);
	sigaction(SIGALRM,&tact,NULL);

}
void init_timer(int timer)
{
	struct itimerval value;
	value.it_value.tv_sec=0;
	value.it_value.tv_usec=1000*timer;
	value.it_interval=value.it_value;  
	setitimer(ITIMER_REAL,&value,NULL);

}	

void init_action_table()
{
	
	action_table[0].Action=&gateway;
	action_table[0].TAG=0x41;
	action_table[0].name="Gateway";
	action_table[1].Action=&config;
	action_table[1].TAG=0x42;
	action_table[1].name="Configuration";
}


void main()
{
	int uart0_filestream = -1;
	int rx_length = 0;    
	uint8_t state=GET_START_BYTE,CC_first=1,CC,CC_old,crc,local_crc;
	uint16_t DeltaF,msg_ptr=0,frame_ID;
	uint8_t Power,Msg_len,ret=0;
	uint8_t tx_buffer[1],rx_buffer[1],message[300];
	uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY );	
	struct timeval start,end;
	long secs_used,micros_used;

	struct termios options;
		tcgetattr(uart0_filestream, &options);
		options.c_cflag = B500000 | CS8 | CLOCAL | CREAD;		//<Set baud rate
		options.c_iflag = IGNPAR;
		options.c_oflag = 0;
		options.c_lflag = 0;
		options.c_cc[VMIN]=1;
		options.c_cc[VTIME]=5;
		tcflush(uart0_filestream, TCIFLUSH);
		tcsetattr(uart0_filestream, TCSANOW, &options);
	mytimeout=0;
	init_sigaction();
	init_action_table();

	if (uart0_filestream != -1)
		{
			
			while(state!=END_STATE)
			{
				// Read up to 255 characters from the port if they are there
				if (state!=SEND_ACK)
				{
					
					rx_length = read(uart0_filestream, (void*)rx_buffer, 1);		//Filestream, buffer to store in, number of bytes to read (max)
					
				}
				if ((mytimeout==1)&&(msg_ptr>0))//if timeout after received at least one char	
				{

					mytimeout=0;
					state=SEND_ACK;
					ret|=MSG_TIMEOUT;	
					printf("mytimeout gateway\n");	
				}


				if ((rx_length > 0) || (state==SEND_ACK)) //no need to wait for a char to send ACK-> must be done immediately
				{
					//Bytes received
					//printf("0x%02X read \n", rx_buffer[0]);
					switch (state)
					{
						
						case GET_START_BYTE:
						gettimeofday(&start,NULL);		
						init_timer(200);
						msg_ptr=0;
						Msg_len=0;
						ret=0;	
						if (rx_buffer[0]==START_BYTE)
						{
							message[msg_ptr++]=rx_buffer[0];
							#ifdef DEBUG
							printf("0x%02X start byte received \n", rx_buffer[0]);
							#endif
							state=GET_FRAMEID;
						}
						else
						{
							#ifdef DEBUG
							printf("0x%02X wrong start byte received \n", rx_buffer[0]);
							#endif
							ret|=WRONG_START_BYTE;	
							state=SEND_ACK;	
						}
						break;
						case GET_FRAMEID:
						frame_ID=rx_buffer[0];
						uint8_t found=0;
						message[msg_ptr++]=rx_buffer[0];
						for (uint8_t i=0;i<MAX_ACTION;i++)
							{
								if (action_table[i].TAG==frame_ID)
									{
										#ifdef DEBUG
										printf("0x%02X byte received name: %s  \n", rx_buffer[0],action_table[i].name);
										#endif
										found=1;
										state=GET_CC;
									}
							}
						if (!found)
						{
							ret|=WRONG_FRAME_ID;
							state=SEND_ACK;
							
							
						}
						
						break;
						case GET_CC:
							CC=rx_buffer[0];
							message[msg_ptr++]=rx_buffer[0];
							if (CC_first)
							{
								CC_first=0;
								#ifdef DEBUG
								printf(" CC byte received: %d \n",CC);
								#endif
								state=GET_DELTAF;
								
								
							}
							else if (CC!=(CC_old+1))
							{
								#ifdef DEBUG
								printf("Bad CC byte %d received %d expected\n",CC,(uint8_t)(CC_old+1));
								#endif
								state=GET_DELTAF;
								ret|=CC_ERROR;   
																	
							}
							else
							{
								
								#ifdef DEBUG
								printf(" CC byte received: %d \n",CC);
								#endif
								state=GET_DELTAF;
								
							}
							CC_old=CC;
							for (uint8_t i=0;i<MAX_ACTION;i++)
							{
								if (action_table[i].TAG==frame_ID)
								{
									action_table[i].Action(uart0_filestream,&ret,&state,message,&msg_ptr);
									
								}
							}
						
						case SEND_ACK:		
								tx_buffer[0]=ret;	
								rx_length = write(uart0_filestream, (void*)tx_buffer, 1);   
								tcflush(uart0_filestream, TCIFLUSH);
								
								gettimeofday(&end,NULL);
								secs_used=(end.tv_sec-start.tv_sec); 
							
								micros_used=((secs_used*1000000)+end.tv_usec)-(start.tv_usec);
								#ifdef DEBUG
								printf("usec: %d\n",micros_used);
								#endif								
								state=GET_START_BYTE;
								mytimeout=0;
								init_timer(0);
						break;
					}
				}
			}
		}

}
