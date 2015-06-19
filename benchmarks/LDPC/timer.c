#include "timer.h"
#include <sys/time.h>

void timer_reset(timing *t){
	t->start = 0;
	t->stop = 0;
	t->total = 0;
	t->elapsed = 0.0;
	
	return;
}

void timer_start(timing *t){
    struct timeval s;

    gettimeofday(&s, 0);
    t->start = (long long)s.tv_sec * 1000000 + (long long)s.tv_usec;
	
	return;
}

void timer_stop(timing *t){
    struct timeval s;
	
    gettimeofday(&s, 0);
    t->stop = (long long)s.tv_sec * 1000000 + (long long)s.tv_usec;
    t->total += t->stop - t->start;
	t->elapsed = (double)t->total / 1000000.0L;
}
