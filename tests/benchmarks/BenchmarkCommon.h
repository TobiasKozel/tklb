#include "../../util/TTimer.h"

#define xstr(s) str(s)
#define str(s) #s

#ifdef TKLB_NO_INTRINSICS
	#define TIMER(text, unit) SectionTimer timer(str(No SIMD text), SectionTimer::unit)
#else
	#define TIMER(text, unit) SectionTimer timer(str(SIMD text took), SectionTimer::unit)
#endif

using namespace tklb;
using namespace std;

