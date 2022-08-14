#include "./TestCommon.hpp"
#include "../src/util/TUuid.h"

int test() {
	const char* invalid = "asdlkasdlkjsadlkjdsalkasjdasdaaaaaaa";
	char invalid2[] = "asdlkhhhhhhhhhhjasdlkj";
	if (tklb::uuid::isValid(invalid)) {
		return 1;
	}
	if (tklb::uuid::isValid(invalid2)) {
		return 2;
	}
	char generate[tklb::uuid::UUIDLength];
	tklb::uuid::generate(generate);
	if (!tklb::uuid::isValid(generate)) {
		return 3;
	}
	return 0;
}
