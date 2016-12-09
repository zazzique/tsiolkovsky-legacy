
#include "windows.h"
#include "Common.h"
#include "Timer.h"

LARGE_INTEGER frequency;
LARGE_INTEGER start_time;

void Timer_Start()
{
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start_time);
}

double Timer_GetCurrentTime()
{
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	return (double)(current_time.QuadPart - start_time.QuadPart) / frequency.QuadPart;
}
