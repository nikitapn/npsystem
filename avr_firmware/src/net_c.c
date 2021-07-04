// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "msvc.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include "../include/avr_firmware/net.h"
#include "../include/avr_firmware/net_dbg.h"
#include "cfg/config.h"

#ifdef MSVC
#	undef MAX485_OUT
#	undef MAX485_IN
#	define MAX485_OUT() 
#	define MAX485_IN() 
#endif // MSVC

#define lo8(val) ((val) & 0xFF)
#define hi8(val) (((val) >> 8) & 0xFF)

#ifdef PC_LINK
#include "tcpip/lan.h"
#endif

#define UBRR_VALUE		(F_CPU / BAUDRATE / 16 - 1)
#define R_DATA_PERIOD (R_DATA_PERIOD_MS / 10)

#define IMMEDIATE_ANSWER	0
#define ANSWER_ON_REQUEST	1
#define NO_ANSWER					2

// Array Section
Frame frame;
struct remote_data rdata[R_DATA_MAX];
// BSS Data Section
static struct remote_data* pbegin_data;
static struct remote_data* pend_data;
static uint8_t* pCurBufPtr;
static uint8_t cur_req_num;

static uint8_t ibbpc;
static uint8_t ac;
//static uint8_t nomm;
static uint8_t tx_count;
static uint8_t rx_len;
static void(*ptf)(void);
static void(*saved_ptf)(void);
static uint8_t saved_ocr;

#ifdef PC_LINK
RuntimeInfoPCLink info;
uint8_t transit;
uint8_t transit_accepted;
#	ifdef PC_LINK_VIRTUAL
extern uint8_t v_buffer[];
#	else
extern uint8_t net_buf[];
#	endif
extern void transit_send(void);
extern void transit_answer(void);
extern void transit_timeout(void);
#else
RuntimeInfoController info;
#endif


#ifndef PC_LINK
static uint8_t write_rd(void);
static uint8_t write_twi(void);
extern uint8_t write_page(void);
extern uint8_t replace_alg(void);
extern uint8_t remove_alg(void);
extern uint8_t stop_alg(uint16_t algAddr);
#endif

static void send_sync_frame(void);
static void start_transmit(void);
static uint8_t send_request(void);
static void read_answer(void);
uint8_t prepare_answer(void);


// ptf functions
static void idle_10(void);
static void idle_40(void);
static void send_answer(void);
static void access_time(void);
static void propagation_delay(void);
static void timeout_for_answer(void);
static void answer_has_been_recived(void);

/*
#ifdef VIRTUAL
volatile uint8_t v_data_recieved;
volatile uint8_t v_data_transmitted;
volatile uint8_t v_busy;
#endif
*/

void net_init(void) {
#ifdef PC_LINK_VIRTUAL
	* (uint8_t*)(0x60) = 0x1A;
#endif // PC_LINK_VIRTUAL


	// LEDs config
	LED_DDR |= (1 << WORK_LED) | (1 << MARKER_LED);

	pbegin_data = pend_data = rdata;

	status = IDLE;
	ibbpc_ev = 0;

	ptf = &idle_40;
	M_OCR = IBBPC_40; // 17.3611111 us - one bit time
	M_TCNT = 0;

	// Init UART
	// PD0 - RX, PD1 - TX, PD2 - SL, PD3 - DIR
	pCurBufPtr = (void*)&frame;
	TX_RX_CTRL_DDR |= (1 << TX_RX_CTRL_PIN);
	TX_RX_CTRL_DDR |= (1 << PD1);
	TX_RX_CTRL_DDR &= ~(1 << PD2);
	M_UBRRL = lo8(UBRR_VALUE);
	M_UBRRH = hi8(UBRR_VALUE);
	M_UCSRA = 0;
	// 1 - stop bit, parity - even, frame length 8 - bit.
	M_UCSRC = UCSRC_VALUE;
	M_UCSRB = RX_ENABLED;
	// External interrupt

	// Нужно захватывать именно начало байта т.к. прерывание по приему байта будет с опозданием.
	MCUCR = (1 << ISC00); //Any logical change on INT0 generates an interrupt request
//	MCUCR = (1 << ISC00) | (1 << ISC01); //The rising edge of INT0 generates an interrupt request
	GIFR = (1 << INTF0);
	GICR = (1 << INT0);
	// Timer 2
	M_TCCR = TIMER_ON;
	M_TIMSK |= ALLOW_TIMER_INT;
}

void net_poll(void) {
	//#ifdef VIRTUAL
	/*	if (v_data_recieved)
		{
			v_data_recieved = 0;

			if (frame.h.fun_num & F_NO_ANSWER)
				v_busy = 0xFF;

			if (prepare_answer() == IMMEDIATE_ANSWER)
			{
				start_transmit();
			}
			v_busy = 0x00;
		} */
		//#else
	if (status & DATA_RECIVED) {
		if (ibbpc_ev >= 1) {
			info.com.e_slave_timeout++;
		}
		goto call_fun;
	}

	if (!ibbpc_ev) return;

	if (ibbpc_ev > 1) {
		// Выход из синхронизированного состояния
		info.com.synclost++;
		M_OCR = IBBPC_40;
		ptf = idle_40;
		M_UCSRB = RX_ENABLED;
		status &= ~SYNC;
		ibbpc_ev = 0;
#ifdef PC_LINK
		//	transit = 0x00;
#endif
		status &= ~HIGH_PRIORITY;
		return;
	}
	ibbpc_ev = 0;
call_fun:
	status &= ~HIGH_PRIORITY;
	ptf();
	//#endif // VIRTUAL
}

static uint8_t inc_ac(void) {
	ac = ac + 1;
	//	if (ac > nomm) ac = 1; // этот счетчик очиститься автоматичеси после синхрофрейма
	if (ac == MY_ADDRESS) {
		//	M_OCR = IBBPC_3 - M_TCNT;
		ptf = &access_time;
		status |= HIGH_PRIORITY;
		return 1;
	}
	return 0;
}

static void idle_40(void) {
	ibbpc = 4;
	if (status & SYNC) {
		if (inc_ac()) {
			return;
		}
	}
	M_OCR = IBBPC_10;
	ptf = idle_10;
}

static void idle_10(void) {
	ibbpc++;
	if (ibbpc >= 50) {
#ifdef MARKER_DEBUG
		//		PORTC ^= (1 << PC5);
#endif
		send_sync_frame();
		return;
	}
	if (status & SYNC) {
		if ((status & LAST_MASTER) && (ibbpc >= 36)) {
#ifdef MARKER_DEBUG
			LED_PORT ^= (1 << MARKER_LED);
#endif
			send_sync_frame();
		} else {
			inc_ac();
		}
	}
}

static void send_sync_frame(void) {
	M_OCR = IBBPC_40;
	ptf = idle_40;
	ac = 0;

	frame.h.sl_addr = MY_ADDRESS | 0x80;
	status |= SYNC | LAST_MASTER;
	frame.h.len = 1;

	start_transmit();
}

static void propagation_delay(void) {
	ptf = saved_ptf;
	M_OCR = saved_ocr - PROPAGATION_DELAY;
	GIFR = (1 << INTF0);
	GICR = (1 << INT0);
	M_UCSRB = RX_ENABLED;
}


static void access_time(void) {
#ifdef DEBUG_CMD
	//	MAKE_DEBUG_CMD(MY_ADDRESS, 0);
#endif // DEBUG_CMD
	uint8_t result;
#ifdef PC_LINK
	if (transit)
	{
		transit_send();
		transit_accepted = 0x01;
		result = 0x01;
	} 
	else
#endif
	{
#ifdef PC_LINK
		transit_accepted = 0x00;
#endif
		result = send_request();
	}
	if (result) {
		M_OCR = IBBPC_30;
		ptf = &timeout_for_answer;
		status |= MASTER_MODE | LAST_MASTER;
		status &= ~DATA_RECIVED;
		start_transmit();
	} else {
		M_OCR = IBBPC_10;
		//	M_TCNT = IBBPC_3;
		ptf = &idle_10;
	}
}

static void timeout_for_answer(void) {
	// Защита от запаздалого ответа
	cli();
	status &= ~MASTER_MODE;
	sei();

	M_OCR = IBBPC_40;
	ptf = &idle_40;

#ifdef PC_LINK
	if (transit_accepted) 
	{
		transit_timeout();
		transit_accepted = transit = 0;
	} 
	else
#endif
	{
		read_answer();
	}
	M_UCSRB = RX_ENABLED;
}

static void answer_has_been_recived(void) {
	ptf = &idle_40;
	status &= ~MASTER_MODE;
#ifdef PC_LINK
	if (transit_accepted) 
	{
		transit_answer();
		transit_accepted = transit = 0;
	} 
	else
#endif
	{
		read_answer();
	}
	M_UCSRB = RX_ENABLED;
}

static void send_answer(void) {
	ptf = &idle_40;
	M_OCR = IBBPC_40;
	if (prepare_answer() == IMMEDIATE_ANSWER) {
		start_transmit();
	} else {
		M_UCSRB = RX_ENABLED;
	}
}

static void start_transmit(void) {
	// Выключаеться контроль за шиной на время передачи
	M_TCCR = TIMER_OFF;
	M_TIFR |= 1 << M_OCF;
	M_TCNT = 0;

	GICR = 0;
	GIFR = (1 << INTF0);

	ibbpc_ev = 0;

	saved_ptf = ptf;
	saved_ocr = M_OCR;

	ptf = &propagation_delay;
	M_OCR = PROPAGATION_DELAY;

	tx_count = frame.h.len;
	pCurBufPtr = (void*)&frame;

	status |= HIGH_PRIORITY;

	MAX485_OUT();
	M_UCSRB = TX_ENABLED;

	M_UDR = (*pCurBufPtr);
}

/* USART, Rx Complete */
ISR(RX_ISR) {
	uint8_t tmp;
	tmp = M_UDR;
	//	if (M_UCSRA & (1 << PE))
	//		return;	
	if (rx_len >= MAX_FRAME_LEN) return;
	rx_len = rx_len + 1;
	(*pCurBufPtr++) = tmp;
}

/* USART, Tx Completed */
ISR(TX_ISR) {
	tx_count = tx_count - 1;
	if (tx_count) {
		M_UDR = *(++pCurBufPtr);
	} else {
#ifdef DEBUG_CMD
		if (frame.h.len > 1) {
			//			MAKE_DEBUG_CMD(MY_ADDRESS, 2);
		}
#endif // DEBUG_CMD
		MAX485_IN();
		M_TCCR = TIMER_ON;
		rx_len = 0;
		pCurBufPtr = (void*)&frame;
	}
}

/* External Interrupt Request 0 */
/* Falling edge - bus has been captured */
/* Rising edge - signal loss */
ISR(INT0_vect) {
	// bus has been captured
	if (!(PIND & (1 << PD2))) {
		ibbpc_ev = 0;
		M_TCCR = TIMER_OFF;
		M_TCNT = 0;
		M_TIFR |= 1 << M_OCF;
		status &= ~DATA_RECIVED;
		return;
	}

	// signal loss
	M_UCSRB = 0;
	M_OCR = IBBPC_40;
	M_TCCR = TIMER_ON;
	ptf = &idle_40;

	// Возможно, одно из устройств передало синхронизирующий фрейм
	if (rx_len == 1 && *((uint8_t*)&frame) & 0x80) {
		// Синхронизируемся...
		ac = 0;
		status &= ~(LAST_MASTER | MASTER_MODE);
		status |= SYNC;
#ifdef MARKER_DEBUG
		LED_PORT ^= (1 << MARKER_LED);
#endif
	}
#ifdef DEBUG_CMD
	//		MAKE_DEBUG_CMD(MY_ADDRESS, 1);
#endif // DEBUG_CMD
	else if (rx_len < 4) {
	}
#ifdef PC_LINK
	else if (transit_accepted && (status & MASTER_MODE))
	{
		ibbpc_ev++;
		ptf = &answer_has_been_recived;
		goto Rx_set;
	}
#endif
	else if (get_crc((void*)&frame, rx_len - 2) == *((uint16_t*)(pCurBufPtr - 2))) {
		// Синхронизируемся каждый раз
		status |= SYNC;
		// Если устройство - мастер ставим флаг "данные получены", выходим
		if (status & MASTER_MODE && frame.h.m_addr == MY_ADDRESS) {
			status |= DATA_RECIVED;
			ptf = &answer_has_been_recived;
			goto Rx_set;
		} else {
			ac = frame.h.m_addr;
			// устанавливаем счетчик
			// если устройство не участвует в обмене он установаиться 2 раза
			// если адресуемое устройство ответит
		}
		// Если устройство не мастер сброс флага предыдущего мастера
		status &= ~LAST_MASTER;
		if (frame.h.sl_addr == MY_ADDRESS) {
			//	if(M_TCNT >= IBBPC_8)
			//	{
			//		ibbpc_ev = 1;
			//	} else {
			//		M_OCR = IBBPC_8 - M_TCNT;
			//	}
			status |= HIGH_PRIORITY;
#ifdef DEBUG_CMD
			//				MAKE_DEBUG_CMD_0060(MY_ADDRESS, M_TCNT);
#endif // DEBUG_CMD
			ptf = &send_answer;
			M_TCNT = 0; // new
			M_OCR = IBBPC_8;
			goto Rx_set;
		}
	} else {
		// bad crc received. do nothing
	}
	M_UCSRB = RX_ENABLED;
Rx_set:
	rx_len = 0;
	pCurBufPtr = (void*)&frame;
}

#ifndef PC_LINK
static uint8_t write_rd(void) {
	pbegin_data = pend_data = rdata;
	memcpy(rdata, frame.r_write_rd.data, frame.r_write_rd.size);
	return write_page();
}

extern uint8_t twi_ix;
extern uint8_t twi_table[];

static uint8_t write_twi(void) {
	twi_ix = 0;
	memcpy(twi_table, frame.r_write_twi.data, frame.r_write_twi.size);
	return write_page();
}
#endif // PC_LINK

uint8_t r_data_cnt;
// Возвращает 1 если запрос был сформирован
// 0 в другом случае
static uint8_t send_request(void) {
	uint8_t size;
	uint16_t pr_addr;
	uint8_t* ptr;
	uint8_t marker;

	// If there are no remote data
	if (!rdata[0].val.device_addr || r_data_cnt) return 0;

	if (pend_data != rdata && (pend_data - 1)->val.marker == M_ARRAY_END) {
		pend_data = rdata;
	}

	pbegin_data = pend_data;

	frame.h.sl_addr = pend_data->val.device_addr;
	frame.h.m_addr = MY_ADDRESS;
	cur_req_num = frame.h.fun_num = (pend_data->val.data_rw_type << 1) | pend_data->val.is_bit;

	if (cur_req_num == REQ_READ_BYTES || cur_req_num == REQ_READ_BITS) {
		pr_addr = 0xffff;
		frame.r_read_bytes.addr_begin = pend_data->val.remote_addr;
		frame.r_read_bytes.nbytes = 0;
		ptr = (uint8_t*)&frame.r_read_bytes.crc;
	} else if (cur_req_num == REQ_WRITE_BYTES) {
		frame.r_write_bytes.len = 0;
		frame.r_write_bytes.addr = pend_data->val.remote_addr;
	}

	do {
		size = pend_data->val.bit_size;
		if (cur_req_num == REQ_READ_BYTES) {
			frame.r_read_bytes.nbytes += size + 1;
		} else if (cur_req_num == REQ_READ_BITS) {
			uint16_t addr = pend_data->val.remote_addr;
			if (pr_addr != addr) {
				pr_addr = addr;
				frame.r_read_bits.nbytes += 1;
			}
		}
		//	case REQ_WRITE_BYTES: {
		//		frame.r_write_bytes.len += size + 1;
		//		for (uint8_t i = 0; i < size + 1; ++i) // plus status
		//		{
		//			(*ptr++) = *((uint8_t*)(pend_data->val.local_addr + i));
		//		}
		//		break;
		//	}
		//	case REQ_WRITE_BITS: {
		//		(*ptr++) = pend_data->val.remote_addr & 0xFF;
		//		(*ptr++) = (pend_data->val.remote_addr >> 8) & 0xFF;
		//		(*ptr++) = pend_data->val.size; // bit number
		//		(*ptr++) = *((uint8_t*)pend_data->val.local_addr) & (1 << pend_data->val.size) ? 0xFF : 0;
		//		(*ptr++) = *((uint8_t*)(pend_data->val.local_addr + 1)) & (1 << pend_data->val.size) ? 0xFF : 0; // status
		//		req_size = req_size + 1;
		//		break;
		//	}
	} while (!(marker = (pend_data++)->val.marker & (M_ARRAY_END | M_LAST_DATA_IN_REQUEST)));

	if (marker == M_ARRAY_END) r_data_cnt = R_DATA_PERIOD;

	frame.h.len = (void*)ptr - (void*)&frame + 2;
	*((uint16_t*)ptr) = get_crc((void*)&frame, frame.h.len - 2);

	return 1;
}

/* Slave mode */
uint8_t prepare_answer(void) {
#ifdef PC_LINK
#	ifdef PC_LINK_VIRTUAL
	Frame* udp_frame = (void*)v_buffer;
#	else
	eth_frame_t* eth_frame = (void*)net_buf;
	ip_packet_t* ip = (void*)(eth_frame->data);
	udp_packet_t* udp = (void*)(ip->data);
	Frame* udp_frame = (void*)udp->data;
#	endif
#endif
	switch (frame.h.fun_num) {
		// Write Bytes
	case REQ_WRITE_BYTES: {
		void* begin_addr = (void*)frame.r_write_bytes.addr;
		memcpy(begin_addr, frame.r_write_bytes.data, frame.r_write_bytes.len);
		return IMMEDIATE_ANSWER;
	}
	// Write Bits
	case REQ_WRITE_BITS: {
		// | bit_addr | bit_mask | bit_value | bit_status |
		for (uint8_t i = 0; i < frame.r_write_bits.nbits; ++i) {
			uint8_t val = *((uint8_t*)frame.r_write_bits.data->bit_addr);

			val &= frame.r_write_bits.data[i].bit_mask;
			val |= frame.r_write_bits.data[i].bit_val | frame.r_write_bits.data[i].bit_status;

			*((uint8_t*)frame.r_write_bits.data->bit_addr) = val;
		}
		return IMMEDIATE_ANSWER;
	}
	// Send Bytes
	case REQ_READ_BYTES:
	case REQ_READ_BITS: {
		// RD сгрупированы и отсортированы по адресу в слейве
		void* begin_addr = (void*)frame.r_read_bytes.addr_begin;
		memcpy(frame.a_read_bytes.data, begin_addr, frame.r_read_bytes.nbytes);
		frame.h.len = sizeof(frame_h) + sizeof(ans_read_bytes) + frame.r_read_bytes.nbytes + 2;
		goto _crc_for_frame;
	}
#ifndef PC_LINK
	case REQ_REINIT_IO: {
		init_io();
		return IMMEDIATE_ANSWER;
	}
	case F_NO_ANSWER | REQ_WRITE_PAGE: {
		status &= ~SYNC;
#ifdef DEBUG_CMD
		MAKE_DEBUG_CMD_0060(MY_ADDRESS, DBG_NET_WRITE_PAGE_START);
#endif
		info.com.result = write_page();
#ifdef DEBUG_CMD
		MAKE_DEBUG_CMD_0060(MY_ADDRESS, DBG_NET_WRITE_PAGE_END);
#endif
		return ANSWER_ON_REQUEST;
	}
	case F_NO_ANSWER | REQ_REPLACE_ALG: {
		status &= ~SYNC;
		info.com.result = replace_alg();
		return ANSWER_ON_REQUEST;
	}
	case F_NO_ANSWER | REQ_REMOVE_ALG: {
		status &= ~SYNC;
		info.com.result = remove_alg();
		return ANSWER_ON_REQUEST;
	}
	case REQ_STOP_ALG: {
		frame.a_stop_alg.result = stop_alg(frame.r_stop_alg.addr);
		frame.h.len = sizeof(frame_h) + sizeof(ans_stop_alg);
		break;
	}
	case F_NO_ANSWER | REQ_WRITE_EEPROM: {
		eeprom_write_block((void*)frame.r_write_epprom.data, (void*)frame.r_write_epprom.addr, frame.r_write_epprom.len);
		info.com.result = 0x00;
		return ANSWER_ON_REQUEST;
	}
	case F_NO_ANSWER | REQ_WRITE_RD: {
		status &= ~SYNC;
		info.com.result = write_rd();
		return ANSWER_ON_REQUEST;
	}
	case F_NO_ANSWER | REQ_WRITE_TWI_TAB: {
		status &= ~SYNC;
		info.com.result = write_twi();
		return ANSWER_ON_REQUEST;
	}
#endif
#ifdef PC_LINK
	case F_NOT_TRANSIT | 0x02: {
		void* begin_addr = (void*)udp_frame->r_read_bytes.addr_begin;
		memcpy(udp_frame->a_read_bytes.data, begin_addr, udp_frame->r_read_bytes.nbytes);
		udp_frame->h.len = sizeof(frame_h) + sizeof(ans_read_bytes) + udp_frame->r_read_bytes.nbytes + 2;
		goto _crc_for_udp;
	}
#endif
	// Write Bit
	case REQ_WRITE_BIT: {
		*((uint8_t*)frame.r_write_bit.bit_addr) &= ~frame.r_write_bit.bit_n;
		*((uint8_t*)frame.r_write_bit.bit_addr) |= frame.r_write_bit.bit_val;
		return IMMEDIATE_ANSWER;
	}
	default:
		return NO_ANSWER;
	}
_crc_for_frame:
	*((uint16_t*)((uint8_t*)&frame + frame.h.len - 2)) =
		get_crc((void*)&frame, frame.h.len - 2);
	return IMMEDIATE_ANSWER;
#ifdef PC_LINK
_crc_for_udp :
	*((uint16_t*)((uint8_t*)udp_frame + udp_frame->h.len - 2)) =
		get_crc((void*)udp_frame, udp_frame->h.len - 2);
	return IMMEDIATE_ANSWER;
#endif
}

/* Master mode */
static void read_answer(void) {
	struct remote_data* p;
	uint8_t ix = 0;
	uint16_t pr_addr = pbegin_data->val.remote_addr;

	if (status & DATA_RECIVED) {
		for (p = pbegin_data; p != pend_data; ++p) {
			p->unreachable_cnt = 0;
			if (cur_req_num == REQ_READ_BYTES) {
				for (uint8_t i = 0; i < p->val.bit_size + 1; i++, ix++) {
					*((uint8_t*)(p->val.local_addr + i)) = frame.a_read_bytes.data[ix];
				}
			} else if (cur_req_num == REQ_READ_BITS) {
				uint16_t addr = (uint16_t)p->val.remote_addr;
				if (pr_addr != addr) {
					ix += 1;
					pr_addr = addr;
				}
				uint8_t bit_mask = (3 << p->val.bit_size);
				uint8_t rem_val = frame.a_read_bits.data[ix] & bit_mask;
				uint8_t val = *((uint8_t*)((uint16_t)p->val.local_addr));

				val &= ~bit_mask;
				val |= rem_val;

				*((uint8_t*)((uint16_t)p->val.local_addr)) = val;
			}
		}
		status &= ~DATA_RECIVED;
	} else {
		if (cur_req_num == REQ_READ_BYTES) {
			while ((p = pbegin_data++) != pend_data) {
				if (p->unreachable_cnt > R_DATA_MAX_TRY) {
					*((uint8_t*)((uint16_t)p->val.local_addr + p->val.bit_size)) = 0x00;
					info.com.rv_error_cnt++;
				} else {
					p->unreachable_cnt++;
					info.com.rv_error_all_cnt++;
				}
			}
		} else if (cur_req_num == REQ_READ_BITS) {
			while ((p = pbegin_data++) != pend_data) {
				if (p->unreachable_cnt > R_DATA_MAX_TRY) {
					*((uint8_t*)((uint16_t)p->val.local_addr)) &= ~(2 << p->val.bit_size);
					info.com.rv_error_cnt++;
				} else {
					p->unreachable_cnt++;
					info.com.rv_error_all_cnt++;
				}
			}
		}
	}
};