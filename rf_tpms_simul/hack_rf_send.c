
#include "/usr/include/libhackrf/hackrf.h"
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#define FD_BUFFER_SIZE (8*1024)
#define DEFAULT_FREQ_HZ (433920000ll) /* 433.92MHz */
#define DEFAULT_SAMPLE_RATE_HZ (4000000) /* 4MHz sample rate */
#define DEFAULT_GAIN	47

static volatile bool do_exit = false;
FILE* file = NULL;
volatile uint32_t byte_count = 0;
uint32_t sample_rate_hz;
int64_t freq_hz;

long int sig_len;
uint8_t *baseband_data;
static long int data_ptr=0;



int tx_callback(hackrf_transfer* transfer) {
	size_t bytes_to_read;
	size_t bytes_read,buffer_ptr;

	uint8_t *buffer=transfer->buffer;	
	//if( file != NULL )
	{
		byte_count += transfer->valid_length;
		bytes_to_read = transfer->valid_length;
		
		for(bytes_read=0;bytes_read<bytes_to_read;bytes_read++)
		{
			//printf("0x%02X ",baseband_data[data_ptr]);
			*(buffer)=baseband_data[data_ptr++]; 
			 
			//printf("0x%02X \n",*(buffer));
			buffer++;
			if (data_ptr>sig_len){break;}


		}
		//bytes_read = fread(transfer->buffer, 1, bytes_to_read, file);
		

		if (bytes_read != bytes_to_read) {
             return -1; /* not repeat mode, end of file */
		} else {
			return 0;
		}
	}/* else  {
        return -1;
    }*/
}

static hackrf_device* device = NULL;

int hack_rf_send(uint8_t *baseband_signal,long int len,uint8_t strength,uint32_t Freq) {

	const char* serial_number = NULL;
	int result;
	int exit_code = 0;
	data_ptr=0;
	baseband_data=baseband_signal;
	sig_len=len;
	//unsigned int  txvga_gain=DEFAULT_GAIN;
	freq_hz = Freq;//DEFAULT_FREQ_HZ;	
	sample_rate_hz = DEFAULT_SAMPLE_RATE_HZ;
	result = hackrf_init();
	
	result = hackrf_open_by_serial(serial_number, &device);
	if( result != HACKRF_SUCCESS ) {
		return HACK_RF_NOT_FOUND;
	}
	
	
	result = hackrf_set_sample_rate(device, sample_rate_hz);
	result = hackrf_set_txvga_gain(device,strength);
	result = hackrf_start_tx(device, tx_callback, NULL);
	result = hackrf_set_freq(device, freq_hz);
	result = hackrf_set_amp_enable(device, 1);
	result = hackrf_set_antenna_enable(device,1);

	while( (hackrf_is_streaming(device) == HACKRF_TRUE) &&
			(do_exit == false) ) 
	{}
	result = hackrf_is_streaming(device);	
	if(device != NULL) {
		result = hackrf_stop_tx(device);
		result = hackrf_close(device);
		hackrf_exit();
	}
	
	return SUCCESS;
}
