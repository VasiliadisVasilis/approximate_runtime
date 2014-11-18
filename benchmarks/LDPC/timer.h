#ifndef _TIMER_H_
#define _TIMER_H_

typedef struct TIMING{
	long long start;
	long long stop;
	long long total;
	double elapsed;
} timing;

void timer_reset(timing *t);

void timer_start(timing *t);

void timer_stop(timing *t);

#endif // _TIMER_H_
