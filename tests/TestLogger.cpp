#define TKLB_IMPL
#include "../src/util/TLogger.hpp"

int main() {
	TKLB_WARN("Hello %s %i %f", "test", 12, 2.4)
	return 0;
}
