FILES=hack_rf_send.c rf_tpms_simul.c baseband.c gateway.c crc.c configuration.c
CC=gcc
CFLAGS=-I.
DEPS=
DEPS=$(FILES:.c=.h) common.h   
OBJ=$(FILES:.c=.o)   

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rf_tpms_simul: $(OBJ)   
	$(CC) -o $@ $^ $(CFLAGS) -lm -lhackrf  

