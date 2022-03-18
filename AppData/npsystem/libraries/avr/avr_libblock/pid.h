#ifndef PID_H_
#define PID_H_

typedef struct
{
	float out;
	uint8_t reserved; // out quality value
	float kp;
	float ki;
	float kd;
	float e_pr;
	float integsum;
	float dt;
} pid_t;

void pid_run(pid_t* data, float sp, float pv) __attribute__((section(".text.pid")));

#endif