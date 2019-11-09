/*	Iakov Sumygin
	2019-11-09

	minimalistic profiler
	BSD-3
*/

#ifndef GLW_PROFILER
#define GLW_PROFILER

#include <string>
#include <stdint.h>
#include <chrono>
#include <thread>
#include <list>
#include <mutex>

// https://github.com/yak32/glw_json
#include "glw_json.h"

#define CLEAR_PERFOMANCE_TIME 2.0f

// enable this for profiling
#define GLW_PROFILE_FUNC(profiler, name)                                                           \
	static glw_counter_t _fc_##name = (profiler).add(name, "", false);         \
	glw_ProfileFunc __glw_profiler_profile_func__(profiler, _fc_##name);

#define GLW_PROFILE_CATEGORY_FUNC(profiler, name, categories)                                      \
	static glw_counter_t _fc_##name = (profiler).add(name, categories, false); \
	glw_ProfileFunc __glw_profiler_profile_func__(profiler, _fc_##name);

#define GLW_PROFILER_THREAD(profiler, name) (profiler).on_thread_started(name)

namespace glw_profiler {

// trace records
// https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview

struct TraceArgs {
	std::string name; // usually thread name
};

typedef uint32_t timestamp_t;

struct Trace {
	const char* name; //  The name of the event, as displayed in Trace Viewer
	const char* cat; // The event categories. This is a comma separated list of categories for the event.
	timestamp_t ts;  // The tracing clock timestamp of the event. The timestamps are provided at
					 // microsecond granularity.
	timestamp_t pid; // The process ID for the process that output this event.
	size_t tid;		 // The thread ID for the thread that output this event.
	const char* ph;  // The event type
	TraceArgs* args;
};

struct TraceObject {
	std::vector<Trace> traceEvents;
	std::string displayTimeUnit = "ms";
};

class Profiler {
public:
	typedef std::chrono::steady_clock clock;
	typedef std::chrono::time_point<clock> time_point;
	typedef std::chrono::microseconds duration;

	struct Counter {
		Counter(const char* _name, const char* _categories, bool _per_frame = false)
			: name(_name), categories(_categories) {}
		std::string name, categories;
	};

	typedef std::vector<Counter*> counters_t;
	typedef std::vector<Trace> traces_t;
	typedef size_t counter_t;

public:
	Profiler() : _tick{}, _mem_frame_time{}, _time_frame{} {
		_fps = 0;
		_fps = 0.0f;
		_frame_count = 0;
		_started = false;
	};

	~Profiler() {
		for (auto* c : _counters)
			delete c;

		for (auto& t : _traces)
			if (t.args)
				delete t.args;
	}

	void start_profiling() {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		assert(!_started && "tracing already started");
		_tracing_start = clock::now();
		_started = true;
	}

	void stop_profiling() {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		assert(_started && "tracing already stopped");
		_started = false;
	}

	void on_thread_started(const char* thread_name) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		auto& t = add_trace_event("M", "thread_name"); // metadata event
		t.name = "thread_name";
		t.args = new TraceArgs;
		t.args->name = thread_name;
	}

	// save traces into json file
	bool save_traces(const char* filename) {
		TraceObject o;
		{
			std::lock_guard<std::recursive_mutex> lock(_mutex);
			o.traceEvents.swap(_traces);
		}
		return json::save_object_to_file(filename, o);
	}

	void begin_frame() {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		_time_frame = std::chrono::duration_cast<duration>(now() - _time_begin_frame);
		_time_begin_frame = now();
	};

	bool end_frame() {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		_frame_count++;
		_frame_count = 0;
		return true;
	};

	counter_t add(const char* name, const char* category = 0, bool per_frame = false) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		_counters.push_back(new Counter(name, category));
		return _counters.size() - 1;
	};

	void start(counter_t i) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (!_started)
			return;

		if (i >= _counters.size()) {
			assert(false);
			return;
		}
		Counter& c = *_counters[i];
		add_trace_event("B", c.name.c_str());
	};

	void stop(counter_t i) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (!_started)
			return;

		if (i >= _counters.size()) {
			assert(false);
			return;
		}
		auto* c = _counters[i];
		add_trace_event("E", c->name.c_str());
	};

	bool is_started() const { return _started;}
	size_t traces_count() const { return _traces.size();}
	const Trace& trace(size_t i) const { return _traces[i];}

protected:
	Trace& add_trace_event(char* type, const char* name, const char* categories = "") {
		_traces.resize(_traces.size() + 1);
		Trace& t = _traces.back();
		t.name = name;
		t.cat = categories;
		t.ph = type;
		t.ts = timestamp(); // The tracing clock timestamp of the event in microseconds
		t.pid = 0;
		t.tid = thread_id();
		t.args = nullptr;
		return t;
	}

	inline time_point now() { return clock::now(); }
	inline timestamp_t timestamp() {
		time_point now = clock::now();
		return (timestamp_t)std::chrono::duration_cast<duration>(now - _tracing_start).count();
	}
	inline size_t thread_id() { return std::hash<std::thread::id>{}(std::this_thread::get_id()); }

protected:
	counters_t _counters;
	time_point _time_begin_frame; // time of frame begin
	duration _time_frame;		  // full time for frame
	size_t _frame_count;		  // count of frames
	float _fps;

	duration _tick;
	time_point _mem_frame_time;
	traces_t _traces;

	bool _started = false;
	time_point _tracing_start;

	std::recursive_mutex _mutex;
	std::list<std::string> _thread_names;
};

class ProfileFunc {
public:
	ProfileFunc(Profiler& p, counter_t counter) : _profiler(p) {
		_counter = counter;
		p.start(_counter);
	};
	~ProfileFunc() { _profiler.stop(_counter); };

private:
	counter_t _counter;
	Profiler& _profiler;
};
} // glw_profiler

namespace json {

template <class T> bool serialize(T& t, glw_TraceArgs& v) {
	bool c = true;
	c = c & SERIALIZE(name);
	return c;
}

template <class T> bool serialize(T& t, glw_Trace& v) {
	bool c = true;
	c = c & SERIALIZE(args);
	c = c & SERIALIZE(cat);
	c = c & SERIALIZE(name);
	c = c & SERIALIZE(ph);
	c = c & SERIALIZE(pid);
	c = c & SERIALIZE(tid);
	c = c & SERIALIZE(ts);
	return c;
}

template <class T> bool serialize(T& t, glw_TraceObject& v) {
	bool c = true;
	c = c & SERIALIZE(displayTimeUnit);
	c = c & SERIALIZE(traceEvents);
	return true;
}
}

#endif // GLW_PROFILER
