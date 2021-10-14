#include "baseband.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include "hack_rf_send.h"

void put_blank(double len);    
uint8_t baseband_signal[10000000];
uint8_t sinus[200],cosinus[200],icosinus[200];
int sin_cos_ptr=0;
int baseband_ptr;
uint8_t getCRC1(uint8_t message[],uint8_t length)
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

void data_init(double samplerate, double deltaf,double baudrate)

{
	int i,I,Q,nQ;
	double t,mysin,mycos,nmycos;
	double tmax=1/(2*baudrate);//2.6e-5;      
	double step=1/samplerate;  

	sin_cos_ptr=0;
	for (t=0;t<tmax;t+=step)
	{
		mysin=MODULATION_RATE*cos((2*3.1415926535)*deltaf*t);
		mycos=(-MODULATION_RATE*sin((2*3.1415926535)*deltaf*t));
		nmycos=(MODULATION_RATE*sin((2*3.1415926535)*deltaf*t));
		I=(uint8_t)(mysin+127);
		Q=(uint8_t)(mycos+127);
		nQ=(uint8_t)(nmycos+127);
		sinus[sin_cos_ptr]=I;
		cosinus[sin_cos_ptr]=Q;
		icosinus[sin_cos_ptr]=nQ;
		sin_cos_ptr++;
	}       
 
}

void data(int val,double samplerate, double deltaf,double baudrate)
{
	int I,Q,nQ;
	double t,mysin,mycos,nmycos;
	double tmax=1/(2*baudrate);//2.6e-5;      
	double step=1/samplerate;  
	double sign=1;
	int my_ptr=0;	
if (val==1)
{
	for (my_ptr=0;my_ptr<sin_cos_ptr;my_ptr++)
	{
		baseband_signal[baseband_ptr++]=sinus[my_ptr];
		baseband_signal[baseband_ptr++]=icosinus[my_ptr];
	}
}
else
{

	for (my_ptr=0;my_ptr<sin_cos_ptr;my_ptr++)
	{
		baseband_signal[baseband_ptr++]=sinus[my_ptr];
		baseband_signal[baseband_ptr++]=cosinus[my_ptr];
	}
}
}
void logic(int val,double samplerate,int deltaf,int baudrate)
{
	if (val==1)
	{
		data(1,samplerate,deltaf,baudrate);
		data(0,samplerate,deltaf,baudrate);
	}
	else
	{
		data(0,samplerate,deltaf,baudrate);
		data(1,samplerate,deltaf,baudrate);
	}
}

void send_byte(uint8_t byte,double samplerate,double deltaf,double baudrate)         
{
	uint8_t i,j=128;
	for(i=0;i<8;i++)   
	{
		if (byte&j)
		{ 
			logic(1,samplerate,deltaf,baudrate);
		}
		else
		{
	
			logic(0,samplerate,deltaf,baudrate);

		}
		j/=2;	
	}

}
void put_blank(double len)
{

char I;
double i;
I=127;

for (i=0;i<len;i++)
{
	baseband_signal[baseband_ptr++]=I;
	baseband_signal[baseband_ptr++]=I;
}
}
void send_message(uint8_t message[],uint8_t len,double samplerate,double deltaf,double baudrate)
{
int i;
data(1,samplerate,deltaf,baudrate);
data(1,samplerate,deltaf,baudrate);
data(1,samplerate,deltaf,baudrate);
data(1,samplerate,deltaf,baudrate);
data(1,samplerate,deltaf,baudrate);
data(1,samplerate,deltaf,baudrate);
data(1,samplerate,deltaf,baudrate);
/*for(i=0;i<3000;i++)
{
	data(1,samplerate,deltaf,baudrate);
}*/

//send_byte(255,samplerate,deltaf,baudrate);
//send_byte(242,samplerate,deltaf,baudrate);
for(i=0;i<len;i++)
{
	send_byte(message[i],samplerate,deltaf,baudrate);
}
//send_byte(getCRC1(message,len),samplerate,deltaf,baudrate);

for(i=0;i<4000;i++)
{
	data(1,samplerate,deltaf,baudrate);
}

put_blank(100000);

} 
uint8_t baseband_gen(double deltaf,uint8_t *message,uint8_t len,uint8_t strength,uint32_t Freq)
{
uint8_t result;
baseband_ptr=0;
double baudrate=19200,samplerate=4000000;
data_init(samplerate,deltaf,baudrate); 
send_message(message,len,samplerate,deltaf,baudrate); 
result=hack_rf_send(baseband_signal,baseband_ptr,strength,Freq);
return result;
}
