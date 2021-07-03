#include "msvc.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <string.h>
#include "../include/avr_firmware/twi.h"

#define TWI_ENABLE ((1 << TWEN) | (1 << TWIE))

#ifdef TWI_GENERAL_CALL
#	define _TWI_ADRR (TWI_ADDR | 1 << TWGCE)
#else
#	define _TWI_ADDR TWI_ADDR
#endif // TWI_GENERAL_CALL

uint8_t twi_buf[TWI_BUFFER_MAX];
volatile uint8_t twi_result;

static uint8_t t_len;
static uint8_t t_header_len;
static uint8_t w_ix;
static uint8_t r_last_index;
static uint8_t r_ix;
static uint8_t twi_master_state;

#ifndef TWI_MASTER_ONLY
uint8_t twi_r_buf[TWI_BUFFER_MAX];
static uint8_t sl_ix;
static void* sl_mem_addr;
static uint8_t sl_flag;
#endif // TWI_MASTER_ONLY

#define TWI_S_READ							0
#define TWI_S_WRITE_ADDR				1
#define TWI_S_WRITE_DATA				2
#define TWI_S_WRITE_NO_ADDRESS	3

void twi_init(void) {
	TWBR = 72; // 100 khz
	TWSR = 0;
	TWAR = _TWI_ADDR;
#ifndef TWI_MASTER_ONLY
	TWCR = TWI_ENABLE | (1 << TWEA);
#endif // MASTER_ONLY
}

void twi_write(twi_req_t* req) {
	twi_result = TWI_BUSY;

	twi_buf[0] = req->dev_addr | TW_WRITE;
	twi_buf[1] = req->wr_value;
	t_len = 2;
	twi_master_state = TWI_S_WRITE_NO_ADDRESS;

	TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTA);
}

void twi_write_bytes(twi_req_t* req) {
	t_header_len = (req->_addrh & 0x80) ? 3 : 2;

	twi_result = TWI_BUSY;

	twi_buf[0] = req->dev_addr | TW_WRITE;
	twi_buf[1] = req->_addrl;
	twi_buf[2] = req->_addrh & 0x7F;

	t_len = req->len + t_header_len;

	twi_master_state = TWI_S_WRITE_ADDR;

	memcpy(twi_buf + t_header_len, req->data, req->len);

	TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTA);
}

void twi_read(twi_req_t* req) {
	twi_result = TWI_BUSY;

	twi_buf[0] = req->dev_addr | TW_READ;
	t_len = 1;

	r_last_index = req->len + 2;

	twi_master_state = TWI_S_READ;

	TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTA);
}

void twi_read_bytes(twi_req_t* req) {
	t_header_len = (req->_addrh & 0x80) ? 3 : 2;
	twi_result = TWI_BUSY;

	twi_buf[0] = req->dev_addr | TW_WRITE;
	twi_buf[1] = req->_addrl;
	twi_buf[2] = req->_addrh & 0x7F;

	t_len = t_header_len;
	r_last_index = req->len + 2;

	twi_master_state = TWI_S_READ;

	TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTA);
}

ISR(TWI_vect)
{
	switch (TWSR & 0xF8)
	{
	case TW_NO_INFO: // No relevant state information available
		break;
	case TW_BUS_ERROR: // Bus error due to an illegal START or STOP condition
		TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTO); 
		// Only the internal hardware is affected, no STOP condition
		// is sent on the bus. In all cases, the bus is released
		// and TWSTO is cleared.
		twi_result = TWI_ERROR;
		break;
	case TW_MT_ARB_LOST: // Arbitration lost in SLA+R or NOT ACK bit
		TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTA); // A START condition will be transmitted when the bus becomes free
		break;
	case TW_START:
	case TW_REP_START:
		if (twi_master_state == TWI_S_WRITE_DATA) {
			TWDR = twi_buf[0];
			TWCR = TWI_ENABLE | (1 << TWINT);
			break;
		}
		w_ix = 0;
		r_ix = 3;
		// Master Transmitter Mode
	case TW_MT_SLA_ACK:
	case TW_MT_DATA_ACK:
		if (w_ix < t_len) {
			TWDR = twi_buf[w_ix++];
			if (twi_master_state == TWI_S_WRITE_ADDR && w_ix == t_header_len) {
				TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTA);
				twi_master_state = TWI_S_WRITE_DATA;
				break;
			}
			TWCR = TWI_ENABLE | (1 << TWINT);
		} else if (twi_master_state == TWI_S_WRITE_DATA) {
			TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTO) | (1 << TWEA);
			twi_result = TWI_SUCCESS;
		} else {
			twi_buf[0] |= TW_READ;
			TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTA);
		}
		break;
		// Master Receiver Mode
	case TW_MR_DATA_ACK:
		twi_buf[r_ix++] = TWDR;
	case TW_MR_SLA_ACK:
		if (r_ix < r_last_index) {
			TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWEA);
		} else {
			TWCR = TWI_ENABLE | (1 << TWINT);
		}
		break;
	case TW_MR_DATA_NACK:
		twi_buf[r_ix] = TWDR;
		TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTO);
		twi_result = TWI_SUCCESS;
		break;
		// MT + MR errors
	case TW_MT_SLA_NACK:
	case TW_MT_DATA_NACK:
	case TW_MR_SLA_NACK:
		TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWSTO);
		twi_result = TWI_ERROR;
		break;
#ifndef TWI_MASTER_ONLY
		// Slave Receiver Mode
	case TW_SR_SLA_ACK:
		sl_ix = 0;
		sl_flag++;
		TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWEA);
		break;
	case TW_SR_DATA_ACK:
		if (sl_flag == 1) {
			((uint8_t*)&sl_mem_addr)[sl_ix++] = TWDR;
		} else {
			twi_r_buf[sl_ix++] = TWDR;
		}
		TWCR = TWI_ENABLE | (1 << TWINT) | (1 << TWEA);
		break;
	case TW_SR_DATA_NACK:
		break;
	case TW_SR_STOP:
		if (sl_flag == 2 && sl_ix) {
			memcpy(sl_mem_addr, twi_r_buf, sl_ix);
			sl_flag = 0;
		}
		TWCR = (1 << TWINT) | (1 << TWEA) | TWI_ENABLE;
		break;
	case TW_SR_ARB_LOST_SLA_ACK:
		sl_ix = 0;
		sl_flag = 0;
		break;
	// Slave Transmitter Mode
	case TW_ST_ARB_LOST_SLA_ACK:
	case TW_ST_SLA_ACK:
		sl_ix = 0;
	case TW_ST_DATA_ACK:
		TWDR = ((uint8_t*)sl_mem_addr)[sl_ix++];
		TWCR = (1 << TWINT) | (1 << TWEA) | TWI_ENABLE;
		break;
	case TW_ST_DATA_NACK:
		sl_ix = 0;
		sl_flag = 0;
		TWCR = (1 << TWINT) | (1 << TWEA) | TWI_ENABLE;
		break;
#endif // MASTER_ONLY
	default:
		break;
	};
}