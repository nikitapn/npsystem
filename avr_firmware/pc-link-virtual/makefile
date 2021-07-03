# pc-link
ADDRESS = 2

CDEFS = -D PC_LINK -D PC_LINK_VIRTUAL -D DEBUG_CMD

# Target file name (without extension).
TARGET = pc-link-virtual

# List C source files here. (C dependencies are automatically generated.)
SRC = ../src/net_c.c ../src/pc_link_v.c

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = ../src/net_s.S ../src/crc.S ../src/zero_memory.S

LDFLAGS = -T../ldscripts/avr4_pc_link.x
LDFLAGS += -Wl,-defsym,__TEXT_REGION_LENGTH__=8192
LDFLAGS += -Wl,-defsym,__DATA_REGION_LENGTH__=1024
LDFLAGS += -Wl,-defsym,__EEPROM_REGION_LENGTH__=512

include ./../makefiles/avr5/pc-link/makefile