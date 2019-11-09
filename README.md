# glw_profiler  ![](https://github.com/yak32/glw_profiler/workflows/status/badge.svg) [![Coverage Status](https://coveralls.io/repos/github/yak32/glw_profiler/badge.svg)](https://coveralls.io/github/yak32/glw_profiler)
Minimalistic profiler for C++ projects.

Header-only, cross-platform and compact profiler in 250 lines.

# Usage
Just drop the header (glw_profiler.h) into your C++ project and include it into a source file

[![ScreenShot](/../screenshots/glw_profiler.png)](/../screenshots/glw_profiler.png)

```C++
#include "glw_profiler.h"
```

# License
Licensed under [BSD](https://opensource.org/licenses/BSD-3-Clause).

### How to build and run tests
```
git clone https://github.com/yak32/glw_profiler.git
cd glw_profiler
mkdir build
cd build
cmake ..
cmake --build . --target install
 ./../install/tests
```
(use Git Bash on Windows)

### Example
```c++
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
	profiler.start_profiling();

	func();

	profiler.stop_profiling();
	profiler.save_traces("traces.json");
	return 1;
}
```
### Dependencies
 C++ 11, standard library, [glw_json](https://github.com/yak32/glw_imgui) (included)

### Performance
Traces are collected in memory, saved after profiling into a json file.

### Tests
 Run cmake on CMakeLists.txt in tests folder.

### Compartibility
  Cross-platform C++11 code, tested on 3 platforms - Ubuntu, Windows and MacOS.


