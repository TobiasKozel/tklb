#define TKLB_IMPL
#include "../src/util/TLogger.hpp"

#ifdef TKLB_NO_STDLIB
	void tklb_print(int level, const char* message) { (void) level; (void) message; }
#endif

int main() {
	TKLB_WARN("Hello %s %i %f", "test", 12, 2.4)
	return 0;
}
