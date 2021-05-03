#define TKLB_MEM_TRACE
#define TKLB_MEM_MONKEY_PATCH
#ifdef _WIN32
	#define TKLB_ASSERT_SEGFAULT
#endif

#include "../src/memory/TMemory.hpp"

using namespace tklb;
using namespace std;
