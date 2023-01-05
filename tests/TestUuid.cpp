#include "./TestCommon.hpp"
#include "../src/util/TUuid.h"

int test() {
	const char* invalid = "asdlkasdlkjsadlkjdsalkasjdasdaaaaaaa";
	const char* invalid2 = "asdlkhhhhhhhhhhjasdlkj";
	if (tklb::uuid::isValid(invalid)) {
		return 1;
	}
	if (tklb::uuid::isValid(invalid2, false)) {
		return 2;
	}

	char generate[tklb::uuid::UUIDLength];
	tklb::uuid::generate(generate, false);
	if (!tklb::uuid::isValid(generate)) {
		return 3;
	}

	char* generate2 = (char*) malloc(tklb::uuid::UUIDLength + 1);
	tklb::uuid::generate(generate2);
	if (!tklb::uuid::isValid(generate2)) {
		return 3;
	}
	return 0;
}
