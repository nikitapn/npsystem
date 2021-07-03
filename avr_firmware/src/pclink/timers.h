#ifndef GTIMERS_H_
#define GTIMERS_H_

#include <stdint.h>

void timers_init(void);
void timers_update(void);
void timer_reset(uint8_t timer_id);
uint16_t timer_get_count(uint8_t timer_id);
uint8_t timer_expired(uint8_t timer_id, uint16_t time);


#endif 

