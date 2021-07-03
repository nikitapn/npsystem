#ifndef __ATMEGA8_HCONFIG_H__
#define __ATMEGA8_HCONFIG_H__

#define TX_RX_CTRL_PORT		PORTD 
#define TX_RX_CTRL_DDR		DDRD 
#define TX_RX_CTRL_PIN		PD3

/*Receiver and transmitter output*/
#define MAX485_IN()		asm volatile("cbi %0, 0x03" :: "I" (_SFR_IO_ADDR(TX_RX_CTRL_PORT)));
#define MAX485_OUT()	asm volatile("sbi %0, 0x03" :: "I" (_SFR_IO_ADDR(TX_RX_CTRL_PORT)));

// Timer 2
#define M_TCCR		TCCR2
// div 64, compare mode
#define TIMER_ON	((1 << CS22) | ( 1 << WGM21 ));
#define TIMER_OFF	0x00

#define M_TCNT	TCNT2
#define M_OCR		OCR2

#define M_TIMSK	TIMSK
#define	ALLOW_TIMER_INT	(1 << OCIE2) 

#define M_TIFR	TIFR
#define M_OCF		OCF2
//

// UART
#define M_UDR			UDR
#define M_UCSRA		UCSRA
#define M_PE			PE
#define M_UCSRB		UCSRB
#define RX_ENABLED	((1<<RXEN) | (1<<RXCIE))
#define TX_ENABLED	((1<<TXEN) | (1<<TXCIE))

#define M_UCSRC			UCSRC
#define UCSRC_VALUE	((1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1) | (1 << UPM1))

#define M_UBRRL		UBRRL
#define M_UBRRH		UBRRH

// Interrupts
#define RX_ISR		USART_RXC_vect
#define TX_ISR		USART_TXC_vect
#define TMR_ISR		TIMER2_COMP_vect


#ifdef PC_LINK
	#define LED_DDR		DDRC
	#define LED_PORT	PORTC
//	#define LED_1 PC0
//	#define LED_2 PC1
	#define LED_3 PC2
	#define LED_4 PC3
	#define LED_5 PC5

// PC0 ?
// PC1 - 1
// PC2 - 2
// PC3 - 3
	#define WORK_LED	 PC1
	#define MARKER_LED PC2
	#define ERROR_LED	 PC3
#else 
	#define LED_DDR			DDRD
	#define LED_PORT		PORTD
	#define WORK_LED		PD4
	#define MARKER_LED	PD5
#endif




#endif // __ATMEGA8_HCONFIG_H__