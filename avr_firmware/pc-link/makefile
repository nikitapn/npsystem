# pc-link
ADDRESS = 1

CDEFS = -D PC_LINK -D TIMERS_MAX=5

# Target file name (without extension).
TARGET = pc-link

# List C source files here. (C dependencies are automatically generated.)
SRC = ../src/pc_link.c ../src/net_c.c ../src/tcpip/enc28j60.c ../src/tcpip/lan.c ../src/pclink/timers.c

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = ../src/net_s.S ../src/crc.S ../src/zero_memory.S

dummy := $(shell mkdir -p obj/tcpip)
dummy := $(shell mkdir -p obj/pclink)

LDFLAGS = -T../ldscripts/avr4_pc_link.x
LDFLAGS += -Wl,-defsym,__TEXT_REGION_LENGTH__=8192
LDFLAGS += -Wl,-defsym,__DATA_REGION_LENGTH__=1024
LDFLAGS += -Wl,-defsym,__EEPROM_REGION_LENGTH__=512

include ./../makefiles/avr5/pc-link/makefile