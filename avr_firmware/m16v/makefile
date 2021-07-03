# atmega16 virtual makefile
ADDRESS = 0
# MCU name
MCU = atmega16
PAGESIZE = 128
#
NRWW_ORG = 0x3C00
NRWW_LEN = 1024

LDFLAGS = -T../ldscripts/avr5.x
LDFLAGS += -Wl,-defsym,__TEXT_REGION_LENGTH__=16384
LDFLAGS += -Wl,-defsym,__DATA_REGION_LENGTH__=1024
LDFLAGS += -Wl,-defsym,__EEPROM_REGION_LENGTH__=512

include ./../makefiles/avr5/virtual/makefile