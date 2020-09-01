#include "../../util/TTimer.h"

#define xstr(s) str(s)
#define str(s) #s

#ifdef TKLB_NO_SIMD
	#ifdef TKLB_SAMPLE_FLOAT
		#define TIMER(unit) SectionTimer timer(__FILE__ str(\tNo SIMD\tfloat\t), SectionTimer::unit)
	#else
		#define TIMER(unit) SectionTimer timer(__FILE__ str(\tNo SIMD\tdouble\t), SectionTimer::unit)
	#endif
#else
	#ifdef TKLB_SAMPLE_FLOAT
		#define TIMER(unit) SectionTimer timer(__FILE__ str(\tSIMD\tfloat\t), SectionTimer::unit)
	#else
		#define TIMER(unit) SectionTimer timer(__FILE__ str(\tSIMD\tdouble\t), SectionTimer::unit)
	#endif
#endif

using namespace tklb;
using namespace std;

