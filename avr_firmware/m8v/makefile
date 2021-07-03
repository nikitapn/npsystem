# atmega8 virtual makefile
ADDRESS = 0
# MCU name
MCU = atmega8
PAGESIZE = 64
#
NRWW_ORG = 0x1c00
NRWW_LEN = 1024

LDFLAGS = -T../ldscripts/avr4.x
LDFLAGS += -Wl,-defsym,__TEXT_REGION_LENGTH__=8192
LDFLAGS += -Wl,-defsym,__DATA_REGION_LENGTH__=1024
LDFLAGS += -Wl,-defsym,__EEPROM_REGION_LENGTH__=512

include ./../makefiles/avr5/virtual/makefile