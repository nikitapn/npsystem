#ifndef CORE_H_
#define CORE_H_

#define SYS_TASK_NUM 2
#define ADC_TASK_DELAY 50
#define I2C_TASK_DELAY 50

#ifndef __ASSEMBLER__

#ifndef NPSYSTEM_FIRMWARE_API_CPP
	#include <avr/io.h>
#endif


#ifdef NPSYSTEM_FIRMWARE_API_CPP
#pragma pack(push, 1)
#endif

typedef struct {
	uint16_t addr;
	uint16_t counter;
} task_t;

typedef struct {
	uint8_t size;
	uint8_t cur_task;
#ifndef NPSYSTEM_FIRMWARE_API_CPP
	task_t t[SPM_PAGESIZE / 4 - 1];
#else
#pragma warning(disable:4200)  
	task_t t[];
#pragma warning(default:4200)  
#endif
	
} tasktab_t;

#ifdef NPSYSTEM_FIRMWARE_API_CPP
typedef enum : unsigned char 
#else
typedef enum
#endif
{
	ADC_OFF,
	ADC_INIT,
	ADC_SET_CHANNEL,
	ADC_CONVERTING,
	ADC_DELAY,
	ADC_DISABLE
} ADC_STATE;
typedef struct {
	ADC_STATE adc_state;
	uint8_t adc_n;
	uint8_t adc_channel[8];
} eeprcfg_t;

#ifdef NPSYSTEM_FIRMWARE_API_CPP
#pragma pack(pop)
#endif

#endif

#endif /* CORE_H_ */