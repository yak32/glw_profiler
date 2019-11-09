#include "glw_profiler.h"

using namespace glw_profiler;

#include "glw_profiler.h"
Profiler profiler;

#define METHOD() GLW_PROFILE_FUNC(g_profiler, __FUNCTION__)

float func(){
	METHOD();
	float sum = 0;
	for(int i=0;i<100000;i++)
		sum += 2.0f;
	return sum;
}

int main(){
	profiler.start_tracing();

	func();

	profiler.stop_tracing();
	profiler.save_traces("traces.json");
	return 1;
}
