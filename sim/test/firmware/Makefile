##
# Project Title
#
# @file
# @version 0.1

CC      = avr-gcc
CFLAGS  = -Os -DF_CPU=16000000UL -mmcu=atmega8

LDFLAGS = -lm
# LDFLAGS += -Tavr4.x
# LDFLAGS += -Wl,-defsym,__TEXT_REGION_LENGTH__=8192
# LDFLAGS += -Wl,-defsym,__DATA_REGION_LENGTH__=1024
# LDFLAGS += -Wl,-defsym,__EEPROM_REGION_LENGTH__=512
# LDFLAGS += -Wl,-defsym,__NRWW_ORIGIN__=0x1c00
# LDFLAGS += -Wl,-defsym,__NRWW_LENGTH__=1024
# LDFLAGS += -Wl,-defsym,__PAZESIZE__=64

OBJC    = avr-objcopy
OBJDUMP = avr-objdump

all: out/add.hex   \
     out/mul.hex   \
	 out/fp32.hex out/fp32.lss \
	 out/add2.hex out/add2.lss

out/%.hex: %.elf
	$(OBJC) -O ihex $< $@

out/%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

%.elf: src/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f out/*.hex out/*.lss

.PHONY: all clean

# end
