#include "Windows.h"

static unsigned __int64 timer_frequency;
static unsigned __int64 base_time;

void set_time_base(void)
{
   QueryPerformanceFrequency((LARGE_INTEGER*)&timer_frequency);
   QueryPerformanceCounter((LARGE_INTEGER*)&base_time);
}

double get_time_in_seconds(void)
{
   unsigned __int64 newtime;
   QueryPerformanceCounter((LARGE_INTEGER*)&newtime);
   return (double)(newtime - base_time) / (double)timer_frequency;
}
