#include "../glw_profiler.h"

using namespace glw_profiler;

Profiler profiler;

#define METHOD() GLW_PROFILE_FUNC(profiler, __FUNCTION__)

float func(){
	METHOD();
	float sum = 0;
	for(int i=0;i<100000;i++)
		sum += 2.0f;
	return sum;
}

int main(){
	profiler.start_profiling();

	func();

	profiler.stop_profiling();
	profiler.save_traces("traces.json");
	return 1;
}
