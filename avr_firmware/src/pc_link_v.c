#include "msvc.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
// #include "cfg/config.h"
#include "../include/avr_firmware/net.h"

uint8_t v_buffer[200];
volatile uint8_t v_data_recieved;
volatile uint8_t v_data_transmitted;
volatile uint8_t v_busy;

extern uint8_t transit;
extern uint8_t transit_accepted;
extern Frame frame;

void extern prepare_answer(void);
void transit_send(void);
void transit_answer(void);
void transit_timeout(void);

#define LED_3 PC3
#define LED_4 PC2
#define LED_5 PC1

void transit_send() {
	memcpy(&frame, v_buffer, transit);
}

void transit_answer() {
	memcpy(v_buffer, &frame, frame.h.len);
	v_data_transmitted = frame.h.len;
}

void transit_timeout() {
	v_buffer[0] = 0xff;
	v_buffer[1] = 'T';
	v_buffer[2] = 0x41;
	v_buffer[3] = 0xbf;
	v_data_transmitted = 4;
}

int main(void) {
	DDRC = 0xff;
	net_init();
	sei();
	while (1) {
		net_poll();
		if (!(status & HIGH_PRIORITY) && !transit) { // net_buf lock 
			if (v_data_recieved) {
				v_data_recieved = 0;
				Frame* vframe = (void*)v_buffer;
				transit = vframe->h.len;
			}
		}
	}
}