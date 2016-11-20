
#include <sys/time.h>
#include "Common.h"
#include "Timer.h"

struct timeval timer_time;
double timer_start_time;

void Timer_Start()
{
    gettimeofday(&timer_time, NULL);
    timer_start_time = (double)timer_time.tv_sec + (0.000001 * (double)timer_time.tv_usec);
}

double Timer_GetCurrentTime()
{
	gettimeofday(&timer_time, NULL);
	return ((double)timer_time.tv_sec + (0.000001 * (double)timer_time.tv_usec) - timer_start_time);
}
