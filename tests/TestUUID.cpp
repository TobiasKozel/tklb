#include "./TestCommon.hpp"
#include "../src/util/TUUID.h"

int test() {
	const char* invalid = "asdlkasdlkjsadlkjdsalkasjdasdaaaaaaa";
	char invalid2[] = "asdlkhhhhhhhhhhjasdlkj";
	if (uuid::isValid(invalid)) {
		return 1;
	}
	if (uuid::isValid(invalid2)) {
		return 2;
	}
	char generate[uuid::UUIDLength];
	uuid::generate(generate);
	if (!uuid::isValid(generate)) {
		return 3;
	}
	return 0;
}
