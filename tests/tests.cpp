#include "../glw_profiler.h"
#include "src/gtest-all.cc"
#include "src/gtest_main.cc"

#include <sstream>

using namespace std;
using namespace glw_profiler;

TEST(ProfilerTest, ProfilerNotStartedByDefault) {
	// set
	Profiler profiler;

	// act
	// check
	EXPECT_FALSE(profiler.is_started());
}

TEST(ProfilerTest, StartProfiler) {
	// set
	Profiler profiler;

	// act
	profiler.start_profiling();

	// check
	EXPECT_TRUE(profiler.is_started());
}

TEST(ProfilerTest, StopProfiler) {
	// set
	Profiler profiler;

	// act
	profiler.start_profiling();
	profiler.stop_profiling();

	// check
	EXPECT_FALSE(profiler.is_started());
}

TEST(ProfilerTest, ProfilerHasNoTracesIfNotStarted) {
	// set
	Profiler profiler;

	// act
	// check
	EXPECT_TRUE(profiler.traces_count()==0);
}

TEST(ProfilerTest, ProfilerThreadName) {
	// set
	Profiler profiler;

	// act
	profiler.on_thread_started("MainThread");

	// check
	EXPECT_TRUE(profiler.traces_count()==1);
	EXPECT_TRUE(strcmp(profiler.trace(0).name, "thread_name")==0);
	EXPECT_TRUE(profiler.trace(0).ts != 0);
	EXPECT_TRUE(profiler.trace(0).args->name=="MainThread");
}

TEST(ProfilerTest, ProfilerNoTracesIfNotStarted) {
	// set
	Profiler profiler;

	// act
	GLW_PROFILE_FUNC(profiler, __FUNCTION__);

	// check
	EXPECT_TRUE(profiler.traces_count()==0);
}

TEST(ProfilerTest, ProfilerTraceAdded) {
	// set
	Profiler profiler;

	// act
	profiler.start_profiling();
	GLW_PROFILE_FUNC(profiler, __FUNCTION__);

	// check
	EXPECT_TRUE(profiler.traces_count()==1);
	EXPECT_TRUE(strcmp(profiler.trace(0).name, __FUNCTION__)==0);
	EXPECT_TRUE(profiler.trace(0).ts != 0);
	EXPECT_TRUE(strcmp(profiler.trace(0).ph, "B")==0);
}

TEST(ProfilerTest, ProfilerFunctionTraceCompleted) {
	// set
	Profiler profiler;

	// act
	profiler.start_profiling();
	{
		GLW_PROFILE_FUNC(profiler, __FUNCTION__);
	}
	// check

	EXPECT_TRUE(profiler.traces_count()==2);
	EXPECT_TRUE(strcmp(profiler.trace(0).name, __FUNCTION__)==0);
	EXPECT_TRUE(strcmp(profiler.trace(0).ph, "B")==0);
	EXPECT_TRUE(profiler.trace(0).ts != 0);
	EXPECT_TRUE(strcmp(profiler.trace(1).name, __FUNCTION__)==0);
	EXPECT_TRUE(strcmp(profiler.trace(1).ph, "E")==0);
	EXPECT_TRUE(profiler.trace(1).ts != 0);
}

TEST(ProfilerTest, ProfilerSaveTracesSucceeded) {
	// set
	Profiler profiler;

	// act
	profiler.start_profiling();
	{
		GLW_PROFILE_FUNC(profiler, __FUNCTION__);
	}
	// check
	EXPECT_TRUE(profiler.save_traces("traces.json"));

}
