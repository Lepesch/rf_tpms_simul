
#ifndef __COMMON_H__ 
#define __COMMON_H__ 
#include <inttypes.h>
enum states
{
	GET_START_BYTE,
	GET_FRAMEID,
	GET_CC,
	GET_DELTAF,
	GET_FREQ,
	GET_POWER,
	GET_MSG_LEN,
	GET_MSG,
	GET_CRC,
	SEND_MSG,
	SEND_ACK,  
	END_STATE 
};

enum error_code
{
	SUCCESS=0,
	WRONG_START_BYTE=1,
	WRONG_FRAME_ID=2,
	CC_ERROR=4,
	SIGNAL_STRENGTH_ERROR=8,
	CRC_ERROR=16,				// 1
	HACK_RF_NOT_FOUND=32,		// 2
	MSG_TIMEOUT=64				// 4
 };

int mytimeout;
/*

Frame to get IP: 47 42 01 05 47 65 74 49 50 36
Frame to reboot: 47 42 01 06 72 65 62 6F 6F 74 A3
Frame to shutdown: 47 42 01 08 73 68 75 74 64 6F 77 6E 6D
test frame: 47 41 00 75 30 20 0A 0A DE AD BE EF 00 00 00 00 01 57
*/

#endif 

