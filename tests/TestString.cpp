#include "./TestCommon.hpp"

#include "../src/types/TString.hpp"

int test() {
	tklb::String<> string = "Test123";
	const char* cstring = "Test123";
	if (string != cstring) {
		return 1;
	}

	const char* cstring2 = "Test1234";
	if (string == cstring2) {
		return 2;
	}

	const char* cstring3 = "Tesa123";
	if (string == cstring3) {
		return 3;
	}

	return 0;
}
