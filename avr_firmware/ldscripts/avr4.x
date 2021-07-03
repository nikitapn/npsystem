OUTPUT_FORMAT("elf32-avr","elf32-avr","elf32-avr")
OUTPUT_ARCH(avr:4)
MEMORY
{
  text   (rx)   : ORIGIN = 0, LENGTH = __TEXT_REGION_LENGTH__ - __NRWW_LENGTH__
  nrww   (rx)   : ORIGIN = __NRWW_ORIGIN__, LENGTH = __NRWW_LENGTH__
  data   (rw!x) : ORIGIN = 0x800062, LENGTH = __DATA_REGION_LENGTH__
  eeprom (rw!x) : ORIGIN = 0x810000, LENGTH = __EEPROM_REGION_LENGTH__
}
SECTIONS
{
  .text   :
  {
    *(.vectors)
    KEEP(*(.vectors))
    /* For data that needs to reside in the lower 64k of progmem.  */
     *(.progmem.gcc*)
    /* PR 13812: Placing the trampolines here gives a better chance
       that they will be in range of the code that uses them.  */
    . = ALIGN(2);
     __trampolines_start = . ;
    /* The jump trampolines for the 16-bit limited relocs will reside here.  */
    *(.trampolines)
     *(.trampolines*)
     __trampolines_end = . ;
    /* avr-libc expects these data to reside in lower 64K. */
     *libprintf_flt.a:*(.progmem.data)
     *libc.a:*(.progmem.data)
     *(.progmem*)
    . = ALIGN(2);
    /* For future tablejump instruction arrays for 3 byte pc devices.
       We don't relax jump/call instructions within these sections.  */
    *(.jumptables)
     *(.jumptables*)
    /* For code that needs to reside in the lower 128k progmem.  */
    *(.lowtext)
     *(.lowtext*)
     __ctors_start = . ;
     *(.ctors)
     __ctors_end = . ;
     __dtors_start = . ;
     *(.dtors)
     __dtors_end = . ;
    KEEP(SORT(*)(.ctors))
    KEEP(SORT(*)(.dtors))
    /* From this point on, we don't bother about wether the insns are
       below or above the 16 bits boundary.  */
    *(.init0)  /* Start here after reset.  */
    KEEP (*(.init0))
    *(.init1)
    KEEP (*(.init1))
    *(.init2)  /* Clear __zero_reg__, set up stack pointer.  */
    KEEP (*(.init2))
    *(.init3) /* Clear all RAM */
    KEEP (*(.init3))
	*(.init4)  /* Initialize data and BSS.  */
    KEEP (*(.init4))
    *(.init5)
    KEEP (*(.init5))
    *(.init6)  /* C++ constructors.  */
    KEEP (*(.init6))
    *(.init7)
    KEEP (*(.init7))
    *(.init8)
    KEEP (*(.init8))
    *(.init9)  /* Call main().  */
    KEEP (*(.init9))
    *(.text)
    . = ALIGN(2);
     *(.text.*)
    . = ALIGN(2);
    *(.fini9)  /* _exit() starts here.  */
    KEEP (*(.fini9))
    *(.fini8)
    KEEP (*(.fini8))
    *(.fini7)
    KEEP (*(.fini7))
    *(.fini6)  /* C++ destructors.  */
    KEEP (*(.fini6))
    *(.fini5)
    KEEP (*(.fini5))
    *(.fini4)
    KEEP (*(.fini4))
    *(.fini3)
    KEEP (*(.fini3))
    *(.fini2)
    KEEP (*(.fini2))
    *(.fini1)
    KEEP (*(.fini1))
    *(.fini0)  /* Infinite loop after program termination.  */
    KEEP (*(.fini0))
	_etext = . ;
  }  > text

  .nrww   :
  {
	_nrww_start = . ;
	*(.nrww.crc)
    . = ALIGN(2);
	*(.nrww.tt)
    . = ALIGN(__PAZESIZE__);
	*(.nrww.rd)
    . = ALIGN(__PAZESIZE__);
	*(.nrww.io)
    . = ALIGN(__PAZESIZE__);
	*(.nrww)
    . = ALIGN(2);
  } > nrww

  .data          :
  {
	 _sys_ram_start =  . ;
     PROVIDE (__data_start = .) ;
    *(.data)
     *(.data*)
    *(.rodata)  /* We need to include .rodata here if gcc is used */
     *(.rodata*) /* with -fdata-sections.  */
    *(.gnu.linkonce.d*)
    . = ALIGN(2);
     _edata = . ;
     PROVIDE (__data_end = .) ;
  }  > data AT> text
  
  .bss  ADDR(.data) + SIZEOF (.data)   : AT (ADDR (.bss))
  {
     PROVIDE (__bss_start = .) ;
    *(.bss)
     *(.bss*)
    *(COMMON)
     PROVIDE (__bss_end = .);
  }  > data

   __data_load_start = LOADADDR(.data);
   __data_load_end = __data_load_start + SIZEOF(.data);
 
 /* with ".tab __data_load_end : ALIGN(__PAZESIZE__)" getting error: nonconstant expression for section alignment */
 .tab __data_load_end :
  {
     . = ALIGN(__PAZESIZE__);
    *(.tab)
    *(.tab*)
	. = ALIGN(__PAZESIZE__);
	_user_space = . ;
  } > tab

 /* Global data not cleared after reset.  */
  .noinit  ADDR(.bss) + SIZEOF (.bss)  :  AT (ADDR (.noinit))
  {
	 _noinit_start = . ;
     PROVIDE (__noinit_start = .) ;
    *(.noinit*)
     PROVIDE (__noinit_end = .) ;
     _end = . ;
	 _sys_ram_end = . ;
  }  > data

  .eeprom  :
  {
    /* See .data above...  */
	KEEP(*(.eeprom.data))
    KEEP(*(.eeprom*))
	_user_eeprom_start = . ;
	FILL(0x00);
	. = . + 200;
	__eeprom_end = . ;
  }  > eeprom
}
