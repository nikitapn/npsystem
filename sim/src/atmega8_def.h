// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "common.h"

struct Atmega8
{
	static constexpr auto CHIP = MC_ATMEGA8;
	static constexpr int FLASH_SIZE = 0x2000;
	static constexpr int RAMEND = 0x045F;
	static constexpr int PAGE_SIZE = 64;
	static constexpr int IO_OFFSET = 0x20;
	static constexpr int EEPROM_SIZE = 0x200;

	static constexpr int TWBR = 0x20;
	static constexpr int TWSR = 0x21;
	static constexpr int TWAR = 0x22;
	static constexpr int TWDR = 0x23;
	static constexpr int ADCL = 0x24;
	static constexpr int ADCH = 0x25;
	static constexpr int ADCSRA = 0x26;
	static constexpr int ADMUX = 0x27;
	static constexpr int ACSR = 0x28;
	static constexpr int UBRRL = 0x29;
	static constexpr int UCSRB = 0x2A;
	static constexpr int UCSRA = 0x2B;
	static constexpr int UDR = 0x2C;
	static constexpr int SPCR = 0x2D;
	static constexpr int SPSR = 0x2E;
	static constexpr int SPDR = 0x2F;
	static constexpr int PIND = 0x30;
	static constexpr int DDRD = 0x31;
	static constexpr int PORTD = 0x32;
	static constexpr int PINC = 0x33;
	static constexpr int DDRC = 0x34;
	static constexpr int PORTC = 0x35;
	static constexpr int PINB = 0x36;
	static constexpr int DDRB = 0x37;
	static constexpr int PORTB = 0x38;
	static constexpr int EECR = 0x3C;
	static constexpr int EEDR = 0x3D;
	static constexpr int EEARL = 0x3E;
	static constexpr int EEARH = 0x3F;
	static constexpr int UBRRH = 0x40;
	static constexpr int WDTCR = 0x41;
	static constexpr int ASSR = 0x42;
	static constexpr int OCR2 = 0x43;
	static constexpr int TCNT2 = 0x44;
	static constexpr int TCCR2 = 0x45;
	static constexpr int ICR1L = 0x46;
	static constexpr int ICR1H = 0x47;
	static constexpr int OCR1BL = 0x48;
	static constexpr int OCR1BH = 0x49;
	static constexpr int OCR1AL = 0x4A;
	static constexpr int OCR1AH = 0x4B;
	static constexpr int TCNT1L = 0x4C;
	static constexpr int TCNT1H = 0x4D;
	static constexpr int TCCR1B = 0x4E;
	static constexpr int TCCR1A = 0x4F;
	static constexpr int SFIOR = 0x50;
	static constexpr int OSCCAL = 0x51;
	static constexpr int TCNT0 = 0x52;
	static constexpr int TCCR0 = 0x53;
	static constexpr int MCUCSR = 0x54;
	static constexpr int MCUCR = 0x55;
	static constexpr int TWCR = 0x56;
	static constexpr int SPMCR = 0x57;
	static constexpr int TIFR = 0x58;
	static constexpr int TIMSK = 0x59;
	static constexpr int GIFR = 0x5A;
	static constexpr int GICR = 0x5B;
	static constexpr int SPL = 0x5D;
	static constexpr int SPH = 0x5E;
	static constexpr int SREG = 0x5F;

	// Interrupts
	static constexpr int External_Interrupt_Request_0 = 0x001;
	static constexpr int External_Interrupt_Request_1 = 0x002;
	static constexpr int TimerCounter2_Compare_Match = 0x003;
	static constexpr int TimerCounter2_Overflow = 0x004;
	static constexpr int TimerCounter1_Capture_Event = 0x005;
	static constexpr int TimerCounter1_Compare_Match_A = 0x006;
	static constexpr int TimerCounter1_Compare_Match_B = 0x007;
	static constexpr int TimerCounter1_Overflow = 0x008;
	static constexpr int TimerCounter0_Overflow = 0x009;
	static constexpr int Serial_Transfer_Complete = 0x00A;
	static constexpr int USART_Rx_Complete = 0x00B;
	static constexpr int USART_Data_Register_Empty = 0x00C;
	static constexpr int USART_Tx_Complete = 0x00D;
	static constexpr int ADC_Conversion_Complete = 0x00E;
	static constexpr int EEPROM_Ready = 0x00F;
	static constexpr int Analog_Comparator = 0x010;
	static constexpr int TwoWire_Serial_Interface = 0x011;
	static constexpr int Store_Program_Memory_Ready = 0x012;

	//static constexpr int TWSR    _SFR_IO8(0x01)
	static constexpr int TWPS0 = 0;
	static constexpr int TWPS1 = 1;
	static constexpr int TWS3 = 3;
	static constexpr int TWS4 = 4;
	static constexpr int TWS5 = 5;
	static constexpr int TWS6 = 6;
	static constexpr int TWS7 = 7;

	//static constexpr int TWAR    _SFR_IO8(0x02)
	static constexpr int TWGCE = 0;
	static constexpr int TWA0 = 1;
	static constexpr int TWA1 = 2;
	static constexpr int TWA2 = 3;
	static constexpr int TWA3 = 4;
	static constexpr int TWA4 = 5;
	static constexpr int TWA5 = 6;
	static constexpr int TWA6 = 7;

	//static constexpr int TWDR    _SFR_IO8(0x03)

	/* Combine ADCL and ADCH */
	//#ifndef __ASSEMBLER__
	//static constexpr int ADC 	_SFR_IO16(0x04)
	//#endif
	//static constexpr int ADCW	_SFR_IO16(0x04)
	//static constexpr int ADCL    _SFR_IO8(0x04)
	//static constexpr int ADCH    _SFR_IO8(0x05)

	//static constexpr int ADCSRA  _SFR_IO8(0x06)
	static constexpr int ADPS0 = 0;
	static constexpr int ADPS1 = 1;
	static constexpr int ADPS2 = 2;
	static constexpr int ADIE = 3;
	static constexpr int ADIF = 4;
	static constexpr int ADATE = 5;
	static constexpr int ADSC = 6;
	static constexpr int ADEN = 7;

	//static constexpr int ADMUX=_SFR_IO8(0x07)
	static constexpr int MUX0 = 0;
	static constexpr int MUX1 = 1;
	static constexpr int MUX2 = 2;
	static constexpr int MUX3 = 3;
	static constexpr int ADLAR = 5;
	static constexpr int REFS0 = 6;
	static constexpr int REFS1 = 7;

	//static constexpr int ACSR= _SFR_IO8(0x08)
	static constexpr int ACIS0 = 0;
	static constexpr int ACIS1 = 1;
	static constexpr int ACIC = 2;
	static constexpr int ACIE = 3;
	static constexpr int ACI = 4;
	static constexpr int ACO = 5;
	static constexpr int ACBG = 6;
	static constexpr int ACD = 7;

	//static constexpr int UBRRL=_SFR_IO8(0x09)
	//static constexpr int UCSRB=_SFR_IO8(0x0A)
	static constexpr int TXB8 = 0;
	static constexpr int RXB8 = 1;
	static constexpr int UCSZ2 = 2;
	static constexpr int TXEN = 3;
	static constexpr int RXEN = 4;
	static constexpr int UDRIE = 5;
	static constexpr int TXCIE = 6;
	static constexpr int RXCIE = 7;

	//static constexpr int UCSRA=_SFR_IO8(0x0B)
	static constexpr int MPCM = 0;
	static constexpr int U2X = 1;
	static constexpr int PE = 2;
	static constexpr int DOR = 3;
	static constexpr int FE = 4;
	static constexpr int UDRE = 5;
	static constexpr int TXC = 6;
	static constexpr int RXC = 7;

	//static constexpr int UDR==_SFR_IO8(0x0C)

	//static constexpr int SPCR= _SFR_IO8(0x0D)
	static constexpr int SPR0 = 0;
	static constexpr int SPR1 = 1;
	static constexpr int CPHA = 2;
	static constexpr int CPOL = 3;
	static constexpr int MSTR = 4;
	static constexpr int DORD = 5;
	static constexpr int SPE = 6;
	static constexpr int SPIE = 7;

	//static constexpr int SPSR= _SFR_IO8(0x0E)
	static constexpr int SPI2X = 0;
	static constexpr int WCOL = 6;
	static constexpr int SPIF = 7;

	//static constexpr int SPDR= _SFR_IO8(0x0F)

	//static constexpr int PIND= _SFR_IO8(0x10)
	static constexpr int PIND0 = 0;
	static constexpr int PIND1 = 1;
	static constexpr int PIND2 = 2;
	static constexpr int PIND3 = 3;
	static constexpr int PIND4 = 4;
	static constexpr int PIND5 = 5;
	static constexpr int PIND6 = 6;
	static constexpr int PIND7 = 7;

	//static constexpr int DDRD= _SFR_IO8(0x11)
	static constexpr int DDD0 = 0;
	static constexpr int DDD1 = 1;
	static constexpr int DDD2 = 2;
	static constexpr int DDD3 = 3;
	static constexpr int DDD4 = 4;
	static constexpr int DDD5 = 5;
	static constexpr int DDD6 = 6;
	static constexpr int DDD7 = 7;

	//static constexpr int PORTD=_SFR_IO8(0x12)
	static constexpr int PD0 = 0;
	static constexpr int PD1 = 1;
	static constexpr int PD2 = 2;
	static constexpr int PD3 = 3;
	static constexpr int PD4 = 4;
	static constexpr int PD5 = 5;
	static constexpr int PD6 = 6;
	static constexpr int PD7 = 7;

	//static constexpr int PINC= _SFR_IO8(0x13)
	static constexpr int PINC0 = 0;
	static constexpr int PINC1 = 1;
	static constexpr int PINC2 = 2;
	static constexpr int PINC3 = 3;
	static constexpr int PINC4 = 4;
	static constexpr int PINC5 = 5;
	static constexpr int PINC6 = 6;

	//static constexpr int DDRC= _SFR_IO8(0x14)
	static constexpr int DDC0 = 0;
	static constexpr int DDC1 = 1;
	static constexpr int DDC2 = 2;
	static constexpr int DDC3 = 3;
	static constexpr int DDC4 = 4;
	static constexpr int DDC5 = 5;
	static constexpr int DDC6 = 6;

	//static constexpr int PORTC=_SFR_IO8(0x15)
	static constexpr int PC0 = 0;
	static constexpr int PC1 = 1;
	static constexpr int PC2 = 2;
	static constexpr int PC3 = 3;
	static constexpr int PC4 = 4;
	static constexpr int PC5 = 5;
	static constexpr int PC6 = 6;

	//static constexpr int PINB= _SFR_IO8(0x16)
	static constexpr int PINB0 = 0;
	static constexpr int PINB1 = 1;
	static constexpr int PINB2 = 2;
	static constexpr int PINB3 = 3;
	static constexpr int PINB4 = 4;
	static constexpr int PINB5 = 5;
	static constexpr int PINB6 = 6;
	static constexpr int PINB7 = 7;

	//static constexpr int DDRB= _SFR_IO8(0x17)
	static constexpr int DDB0 = 0;
	static constexpr int DDB1 = 1;
	static constexpr int DDB2 = 2;
	static constexpr int DDB3 = 3;
	static constexpr int DDB4 = 4;
	static constexpr int DDB5 = 5;
	static constexpr int DDB6 = 6;
	static constexpr int DDB7 = 7;

	//static constexpr int PORTB=_SFR_IO8(0x18)
	static constexpr int PB0 = 0;
	static constexpr int PB1 = 1;
	static constexpr int PB2 = 2;
	static constexpr int PB3 = 3;
	static constexpr int PB4 = 4;
	static constexpr int PB5 = 5;
	static constexpr int PB6 = 6;
	static constexpr int PB7 = 7;


	/* EEPROM Control Register */
	//static constexpr int EECR	_SFR_IO8(0x1C)
	static constexpr int EERE = 0;
	static constexpr int EEWE = 1;
	static constexpr int EEMWE = 2;
	static constexpr int EERIE = 3;

	/* EEPROM Data Register */
	//static constexpr int EEDR	_SFR_IO8(0x1D)

	/* EEPROM Address Register */
	//static constexpr int EEAR	_SFR_IO16(0x1E)
	//static constexpr int EEARL	_SFR_IO8(0x1E)
	//static constexpr int EEARH	_SFR_IO8(0x1F)

	//static constexpr int UCSRC=_SFR_IO8(0x20)
	static constexpr int UCPOL = 0;
	static constexpr int UCSZ0 = 1;
	static constexpr int UCSZ1 = 2;
	static constexpr int USBS = 3;
	static constexpr int UPM0 = 4;
	static constexpr int UPM1 = 5;
	static constexpr int UMSEL = 6;
	static constexpr int URSEL = 7;

	//static constexpr int UBRRH=_SFR_IO8(0x20)
	//	static constexpr int URSEL = 7;

	//static constexpr int WDTCR=_SFR_IO8(0x21)
	static constexpr int WDP0 = 0;
	static constexpr int WDP1 = 1;
	static constexpr int WDP2 = 2;
	static constexpr int WDE = 3;
	static constexpr int WDTOE = 4;

	//static constexpr int ASSR= _SFR_IO8(0x22)
	static constexpr int TCR2UB = 0;
	static constexpr int OCR2UB = 1;
	static constexpr int TCN2UB = 2;
	static constexpr int AS2 = 3;

	//static constexpr int OCR2= _SFR_IO8(0x23)

	//static constexpr int TCNT2=_SFR_IO8(0x24)

	//static constexpr int TCCR2=_SFR_IO8(0x25)
	static constexpr int CS20 = 0;
	static constexpr int CS21 = 1;
	static constexpr int CS22 = 2;
	static constexpr int WGM21 = 3;
	static constexpr int COM20 = 4;
	static constexpr int COM21 = 5;
	static constexpr int WGM20 = 6;
	static constexpr int FOC2 = 7;

	/* Combine ICR1L and ICR1H */
	//static constexpr int ICR1= _SFR_IO16(0x26)

	//static constexpr int ICR1L=_SFR_IO8(0x26)
	//static constexpr int ICR1H=_SFR_IO8(0x27)

	/* Combine OCR1BL and OCR1BH */
	//static constexpr int OCR1B=_SFR_IO16(0x28)

	//static constexpr int OCR1BL=_SFR_IO8(0x28)
	//static constexpr int OCR1BH=_SFR_IO8(0x29)

	/* Combine OCR1AL and OCR1AH */
	//static constexpr int OCR1A=_SFR_IO16(0x2A)

	//static constexpr int OCR1AL=_SFR_IO8(0x2A)
	//static constexpr int OCR1AH=_SFR_IO8(0x2B)

	/* Combine TCNT1L and TCNT1H */
	//static constexpr int TCNT1=_SFR_IO16(0x2C)

	//static constexpr int TCNT1L=_SFR_IO8(0x2C)
	//static constexpr int TCNT1H=_SFR_IO8(0x2D)

	//static constexpr int TCCR1B=_SFR_IO8(0x2E)
	static constexpr int CS10 = 0;
	static constexpr int CS11 = 1;
	static constexpr int CS12 = 2;
	static constexpr int WGM12 = 3;
	static constexpr int WGM13 = 4;
	static constexpr int ICES1 = 6;
	static constexpr int ICNC1 = 7;

	//static constexpr int TCCR1A=_SFR_IO8(0x2F)
	static constexpr int WGM10 = 0;
	static constexpr int WGM11 = 1;
	static constexpr int FOC1B = 2;
	static constexpr int FOC1A = 3;
	static constexpr int COM1B0 = 4;
	static constexpr int COM1B1 = 5;
	static constexpr int COM1A0 = 6;
	static constexpr int COM1A1 = 7;

	/*
	The ADHSM bit has been removed from all documentation,
	as being not needed at all since the comparator has proven
	to be fast enough even without feeding it more power.
	*/

	//static constexpr int SFIOR=_SFR_IO8(0x30)
	static constexpr int PSR10 = 0;
	static constexpr int PSR2 = 1;
	static constexpr int PUD = 2;
	static constexpr int ACME = 3;

	//static constexpr int OSCCAL=_SFR_IO8(0x31)

	//static constexpr int OCDR= _SFR_IO8(0x31)

	//static constexpr int TCNT0=_SFR_IO8(0x32)

	//static constexpr int TCCR0=_SFR_IO8(0x33)
	static constexpr int CS00 = 0;
	static constexpr int CS01 = 1;
	static constexpr int CS02 = 2;

	//static constexpr int MCUCSR=_SFR_IO8(0x34)
	static constexpr int PORF = 0;
	static constexpr int EXTRF = 1;
	static constexpr int BORF = 2;
	static constexpr int WDRF = 3;
	static constexpr int JTRF = 4;
	static constexpr int ISC2 = 6;
	static constexpr int JTD = 7;

	//static constexpr int MCUCR=_SFR_IO8(0x35)
	static constexpr int ISC00 = 0;
	static constexpr int ISC01 = 1;
	static constexpr int ISC10 = 2;
	static constexpr int ISC11 = 3;
	static constexpr int SM0 = 4;
	static constexpr int SM1 = 5;
	static constexpr int SE = 6;
	static constexpr int SM2 = 7;

	//static constexpr int TWCR= _SFR_IO8(0x36)
	static constexpr int TWIE = 0;
	static constexpr int TWEN = 2;
	static constexpr int TWWC = 3;
	static constexpr int TWSTO = 4;
	static constexpr int TWSTA = 5;
	static constexpr int TWEA = 6;
	static constexpr int TWINT = 7;

	//static constexpr int SPMCR=_SFR_IO8(0x37)
	static constexpr int SPMEN = 0;
	static constexpr int PGERS = 1;
	static constexpr int PGWRT = 2;
	static constexpr int BLBSET = 3;
	static constexpr int RWWSRE = 4;
	static constexpr int RWWSB = 6;
	static constexpr int SPMIE = 7;

	//static constexpr int TIFR= _SFR_IO8(0x38)
	static constexpr int TOV0 = 0;
	static constexpr int OCF0 = 1;
	static constexpr int TOV1 = 2;
	static constexpr int OCF1B = 3;
	static constexpr int OCF1A = 4;
	static constexpr int ICF1 = 5;
	static constexpr int TOV2 = 6;
	static constexpr int OCF2 = 7;

	//static constexpr int TIMSK=_SFR_IO8(0x39)
	static constexpr int TOIE0 = 0;
	static constexpr int TOIE1 = 2;
	static constexpr int OCIE1B = 3;
	static constexpr int OCIE1A = 4;
	static constexpr int TICIE1 = 5;
	static constexpr int TOIE2 = 6;
	static constexpr int OCIE2 = 7;

	//static constexpr int GIFR= _SFR_IO8(0x3A)
	static constexpr int INTF0 = 6;
	static constexpr int INTF1 = 7;

	//static constexpr int GICR= _SFR_IO8(0x3B)
	static constexpr int IVCE = 0;
	static constexpr int IVSEL = 1;
	static constexpr int INT0 = 6;
	static constexpr int INT1 = 7;
};