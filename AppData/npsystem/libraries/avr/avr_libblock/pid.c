#include <avr/io.h>
#include <math.h>
#include "pid.h"

#define out			data->out
#define kp			data->kp
#define ki			data->ki
#define kd			data->kd
#define e_pr		data->e_pr
#define integsum	data->integsum
#define dt			data->dt

void pid_run(pid_t* data, float sp, float pv) {
	float e = sp - pv;
	out = kp * e + kd * (e - e_pr);
	e_pr = e;
}