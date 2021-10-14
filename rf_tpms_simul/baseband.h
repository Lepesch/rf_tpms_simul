#ifndef __BASEBANDH__
#define __BASEBANDH__
#include <inttypes.h>
uint8_t baseband_gen(double deltaf,uint8_t *message,uint8_t len,uint8_t strength,uint32_t Freq);
#define MODULATION_RATE	40




#endif
