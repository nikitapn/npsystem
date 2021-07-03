# makefile controller
CDEFS += -D TWI_ADDR=0x10
CDEFS += -D TWI_MASTER_ONLY

SRC = ../src/net_c.c ../src/core_c.c ../src/twi.c
ASRC = ../src/net_s.S ../src/crc.S ../src/tables.S ../src/self_prog.S ../src/core_s.S ../src/zero_memory.S

include ./../makefiles/avr5/makefile