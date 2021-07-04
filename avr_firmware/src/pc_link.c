// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "msvc.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "cfg/config.h"
#include "../include/avr_firmware/net.h"
#include "tcpip/lan.h"
#include "pclink/timers.h"

#define G_TIMER_WORK_LED	0
#define G_WATCH_ENC28J69	1

#ifndef PC_LINK
	#error "PC_LINK is not defined"
#endif

#define PORT 28040

extern uint8_t transit;
extern uint8_t transit_accepted;

extern Frame frame;

extern uint8_t net_buf[];

void extern prepare_answer(void);

void transit_send(void);
void transit_answer(void);
void transit_timeout(void);

// при посылке 128 байт - выходит из синхронизации
void udp_packet(eth_frame_t *eth_frame, uint16_t len) {
	ip_packet_t *ip = (void*)(eth_frame->data);
	udp_packet_t *udp = (void*)(ip->data);
	Frame* udp_frame = (void*)udp->data;

	if(udp->to_port != htons(PORT)) return;
	
	if (udp_frame->h.fun_num & F_NOT_TRANSIT) {
		prepare_answer();
		udp_reply(eth_frame, udp_frame->h.len);
	} else {
		transit = len;
	}
}

void transit_send() {
	eth_frame_t* eth_frame = (void*)net_buf;
	ip_packet_t* ip = (void*)(eth_frame->data);
	udp_packet_t* udp = (void*)(ip->data);
	
	memcpy(&frame, udp->data, transit);
}

void transit_answer() {
	eth_frame_t* eth_frame = (void*)net_buf;
	ip_packet_t* ip = (void*)(eth_frame->data);
	udp_packet_t* udp = (void*)(ip->data);
	
	memcpy(udp->data, &frame, frame.h.len);
	udp_reply(eth_frame, frame.h.len);
}

void transit_timeout() {
	eth_frame_t* eth_frame = (void*)net_buf;
	ip_packet_t* ip = (void*)(eth_frame->data);
	udp_packet_t* udp = (void*)(ip->data);
	
	udp->data[0] = 0xff;
	udp->data[1] = 'T';
	udp->data[2] = 0x41;
	udp->data[3] = 0xbf;
	
	udp_reply(eth_frame, 4);
}


void status_led_proc(void) {
	if (timer_expired(G_TIMER_WORK_LED, 500)) {
		LED_PORT ^= (1 << WORK_LED);
	}
}

void watch_enc28j60(void) {
	if (timer_expired(G_WATCH_ENC28J69, 500)) {
		enc28cj60_reinit_if_failed();
	}
}


int main(void) {
	LED_DDR = 0xff;

	timers_init();
	net_init();
	lan_init();

	sei();

	while (1) {
		timers_update();
		net_poll();
		if (!(status & HIGH_PRIORITY) && !transit) lan_poll();
		status_led_proc();
		watch_enc28j60();
	}
}